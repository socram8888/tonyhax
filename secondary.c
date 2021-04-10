
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "bios.h"
#include "cdrom.h"
#include "cfgparse.h"
#include "crc.h"
#include "debugscreen.h"
#include "gpu.h"
#include "patcher.h"
#include "io.h"

// Set to zero unless you are using an emulator or have a physical UART on the PS1, else it'll freeze
const uint32_t tty_enabled = 0;

// Loading address of tonyhax, provided by the secondary.ld linker script
extern uint8_t __RO_START__, __DATA_START__, __BSS_START__, __BSS_END__;

// Buffer right before this executable
uint8_t * const data_buffer = (uint8_t *) 0x801F9800;

// Kernel developer
const char * const KERNEL_AUTHOR = (const char *) 0xBFC0012C;

// Version string
const char * const VERSION_STRING = (const char *) 0xBFC7FF32;

void reinit_kernel() {
	// Disable interrupts
	EnterCriticalSection();

	// The following is adapted from the WarmBoot call

	/*
	 * Check if a PS1 by testing the developer credit string.
	 *
	 * PS1 have in this field one of the following:
	 *  - CEX-3000 KT-3  by K.S
	 *  - CEX-3000/1001/1002 by K.S
	 *  - CEX-3000/1001/1002 by K.S
	 *
	 * PS2 have "PS compatible mode by M.T"
	 */
	if (strncmp(KERNEL_AUTHOR, "CEX-", 4) == 0) {
		// Restore part of the kernel memory
		memcpy((uint8_t *) 0xA0000500, (const uint8_t *) 0xBFC10000, 0x8BF0);

		// Call it to restore everything that it needs to
		((void (*)(void)) 0xA0000500)();

		// Restore call tables
		memcpy((uint8_t *)      0x200, (const uint8_t *) 0xBFC04300, 0x300);
	}

	// Restore A, B and C tables
	init_a0_b0_c0_vectors();

	// Fix A table
	AdjustA0Table();

	// Install default exception handlers
	InstallExceptionHandlers();

	// Clear interrupts and mask
	*I_STAT = 0;
	*I_MASK = 0;

	// Setup devices
	InstallDevices(tty_enabled);

	// Initialize kernel memory
	SysInitMemory(0xA000E000, 0x2000);

	// Enqueue syscall handler with priority 0
	EnqueueSyscallHandler(0);

	// Initialize default interrupt
	InitDefInt(3);

	// Setup timer and Vblank with priority 1
	EnqueueTimerAndVblankIrqs(1);

	// End of code adapted

	// Re-enable interrupts
	ExitCriticalSection();
}

bool test_integrity() {
	/*
	 * The integrity consists of a CRC32 inside the title frame after the title name,
	 * at offset 0x4C.
	 * This hash is calculated over all the read-only payload.
	 */
	uint32_t correct_value = *((uint32_t * ) (&__RO_START__ - 0x100 + 0x4C));
	uint32_t calc_value = crc32(&__RO_START__, &__DATA_START__ - &__RO_START__);

	bool ok = correct_value == calc_value;
	debug_write("Integrity check %sed", ok ? "pass" : "fail");

	return ok;
}

void log_bios_version() {
	/*
	 * "System ROM Version 4.5 05/25/00 A"
	 * By adding 11 we get to Version, which we'll compare as a shortcut
	 */
	const char * version;

	if (strncmp(VERSION_STRING + 11, "Version", 7) == 0) {
		version = VERSION_STRING + 19;
	} else {
		version = "1.0 or older";
	}

	debug_write("BIOS: v%s", version);
}

bool backdoor_cmd(uint_fast8_t cmd, const char * string) {
	uint8_t cd_reply[16];

	// Send command
	cd_command(cmd, (const uint8_t *) string, strlen(string));

	// Check if INT5, else fail
	if (cd_wait_int() != 5) {
		return false;
	}

	// Check length
	uint_fast8_t reply_len = cd_read_reply(cd_reply);
	if (reply_len != 2) {
		return false;
	}

	// Check there is an error flagged
	if (!(cd_reply[0] & 0x01)) {
		return false;
	}

	// Check error code
	if (cd_reply[1] != 0x40) {
		return false;
	}

	return true;
}

bool unlock_drive() {
	uint8_t cd_reply[16];

	// Run "GetRegion" test
	uint8_t test = CD_TEST_REGION;
	cd_command(CD_CMD_TEST, &test, 1);

	// Should succeed with 3
	if (cd_wait_int() != 3) {
		debug_write("Region read failed");
		return false;
	}

	// Read actual region text and null terminate it
	int len = cd_read_reply(cd_reply);
	cd_reply[len] = 0;

	// Compare which is the fifth string we have to send to the backdoor
	const char * region_name;
	const char * p5_localized;
	if (strcmp((char *) cd_reply, "for Europe") == 0) {
		region_name = "European";
		p5_localized = "(Europe)";
	} else if (strcmp((char *) cd_reply, "for U/C") == 0) {
		region_name = "American";
		p5_localized = "of America";
	} else if (strcmp((char *) cd_reply, "for NETEU") == 0) {
		region_name = "NetYaroze (EU)";
		p5_localized = "World wide";
	} else if (strcmp((char *) cd_reply, "for NETNA") == 0) {
		region_name = "NetYaroze (US)";
		p5_localized = "World wide";
	} else {
		// +4 to skip past "for "
		debug_write("Unsup. region: %s", (char *) (cd_reply + 4));
		return false;
	}

	debug_write("Drive region: %s", region_name);

	// Note the kernel's implementation of strlen returns 0 for nulls.
	if (
			!backdoor_cmd(0x50, NULL) ||
			!backdoor_cmd(0x51, "Licensed by") ||
			!backdoor_cmd(0x52, "Sony") ||
			!backdoor_cmd(0x53, "Computer") ||
			!backdoor_cmd(0x54, "Entertainment") ||
			!backdoor_cmd(0x55, p5_localized) ||
			!backdoor_cmd(0x56, NULL)
	) {
		debug_write("Backdoor failed");
		return false;
	}

	return true;
}

void wait_lid_status(bool open) {
	uint8_t cd_reply[16];

	uint8_t expected = open ? 0x10 : 0x00;
	do {
		// Issue Getstat command
		// We cannot issue the BIOS CD commands yet because we haven't called CdInit
		cd_command(CD_CMD_GETSTAT, NULL, 0);

		// Always returns 3, no need to check
		cd_wait_int();

		// Always returns one, no need to check either
		cd_read_reply(cd_reply);

	} while ((cd_reply[0] & 0x10) != expected);
}

void try_boot_cd() {
	int32_t fd, read;

	debug_write("Swap CD now");
	wait_lid_status(true);
	wait_lid_status(false);

	debug_write("Initializing CD");
	CdInit();

	// Defaults if no SYSTEM.CNF file exists
	uint32_t tcb = 4;
	uint32_t event = 16;
	uint32_t stacktop = 0x801FFF00;
	const char * bootfile = "cdrom:PSX.EXE;1";

	char bootfilebuf[32];
	debug_write("Loading SYSTEM.CNF");

	fd = FileOpen("cdrom:SYSTEM.CNF;1", FILE_READ);
	if (fd > 0) {
		read = FileRead(fd, data_buffer, 2048);
		FileClose(fd);

		if (read == -1) {
			debug_write("Read error %x", GetLastError());
			return;
		}

		// Null terminate
		data_buffer[read] = '\0';

		config_get_hex((char *) data_buffer, "TCB", &tcb);
		config_get_hex((char *) data_buffer, "EVENT", &event);
		config_get_hex((char *) data_buffer, "STACK", &stacktop);
		if (config_get_string((char *) data_buffer, "BOOT", bootfilebuf)) {
			bootfile = bootfilebuf;
		}

	} else {
		uint32_t errorCode = GetLastError();
		if (errorCode != FILEERR_NOT_FOUND) {
			debug_write("Open error %x", errorCode);
			return;
		}

		debug_write("Not found");
	}

	// Use string format to reduce ROM usage
	debug_write("%s = %x", "TCB", tcb);
	debug_write("%s = %x", "EVENT", event);
	debug_write("%s = %x", "STACK", stacktop);
	debug_write("%s = %s", "BOOT", bootfile);

	debug_write("Configuring kernel");
	SetConf(event, tcb, stacktop);

	debug_write("Loading executable");
	fd = FileOpen(bootfile, FILE_READ);
	if (fd <= 0) {
		debug_write("Open error %x", GetLastError());
	}

	read = FileRead(fd, data_buffer, 2048);
	FileClose(fd);

	if (read != 2048) {
		debug_write("Read error %x", GetLastError());
		return;
	}

	// On European games, at 0x4C there is a string that "Sony Computer Entertainment Inc. for Europe area"
	bool is_european_game = strncmp("Europe", (char *) (data_buffer + 0x71), 6) == 0;

	exe_header_t * exe_header = (exe_header_t *) (data_buffer + 0x10);

	// If the file overlaps tonyhax, we will use the unstable LoadAndExecute function
	// since that's all we can do.
	if (exe_header->load_addr + exe_header->load_size >= data_buffer) {
		debug_write("Won't fit. Using BIOS.");
		LoadAndExecute(bootfile, exe_header->initial_sp_base, exe_header->initial_sp_offset);
		return;
	}

	if (!LoadExeFile(bootfile, data_buffer)) {
		debug_write("Loading failed");
		return;
	}

	patch_game(exe_header);

	bool is_pal = gpu_is_pal();
	if (is_pal != is_european_game) {
		debug_write("Switching to %s", is_european_game ? "PAL" : "NTSC");
		gpu_init_bios(is_european_game);
	}

	debug_write("Starting");

	// Games from WarmBoot start with interrupts disabled
	EnterCriticalSection();

	DoExecute(data_buffer, 0, 0);
}

void main() {
	// Undo all possible fuckeries during exploiting
	reinit_kernel();

	// Initialize debug screen
	debug_init();

	if (!test_integrity()) {
		return;
	}

	log_bios_version();

	for (int i = 0; i < 30; i++) {
		debug_write("Line %x", i);
	}

	debug_write("Resetting drive");
	if (!cd_drive_init()) {
		debug_write("Reset failed");
		return;
	}

	debug_write("Unlocking drive");
	if (!unlock_drive()) {
		return;
	}

	while (1) {
		try_boot_cd();

		debug_write("Reinitializing kernel");
		reinit_kernel();
	}
}

void __attribute__((section(".start"))) start() {
	// Clear BSS
	bzero(&__BSS_START__, &__BSS_END__ - &__BSS_START__);

	main();

	while(1);
}
