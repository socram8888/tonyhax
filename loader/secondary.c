
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "str.h"
#include "audio.h"
#include "bios.h"
#include "cdrom.h"
#include "cfgparse.h"
#include "crc.h"
#include "debugscreen.h"
#include "gpu.h"
#include "patcher.h"
#include "integrity.h"
#include "io.h"

// Loading address of tonyhax, provided by the secondary.ld linker script
extern uint8_t __RO_START__, __BSS_START__, __BSS_END__;

void log_bios_version() {
	/*
	 * "System ROM Version 4.5 05/25/00 A"
	 * By adding 11 we get to Version, which we'll compare as a shortcut
	 */
	const char * version;

	if (strncmp(BIOS_VERSION + 11, "Version", 7) == 0) {
		version = BIOS_VERSION + 19;
	} else {
		version = "1.0 or older";
	}

	debug_write("Console: %s", bios_is_ps1() ? "PS1": "PS2");
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
		debug_write("Unsupported region: %s", (char *) (cd_reply + 4));
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
	int32_t read;

	debug_write("Swap CD now");
	wait_lid_status(true);
	wait_lid_status(false);

	debug_write("Initializing CD");
	if (!CdInit()) {
		debug_write("Init failed");
		return;
	}

	/*
	 * Use the space the BIOS has allocated for reading CD sectors.
	 *
	 * The English translation of Mizzurna Falls (J) (SLPS-01783) depends on the header being
	 * present here (see issue #95 in GitHub).
	 *
	 * This address varies between PS1 and PS2.
	 */
	uint8_t * data_buffer = (uint8_t *) (bios_is_ps1() ? 0xA000B070 : 0xA000A8D0);

	debug_write("Checking game region");
	if (CdReadSector(1, 4, data_buffer) != 1) {
		debug_write("Failed to read sector");
		return;
	}

	const char * game_region;
	bool game_is_pal = false;
	/*
	 * EU: "          Licensed  by          Sony Computer Entertainment Euro pe   "
	 * US: "          Licensed  by          Sony Computer Entertainment Amer  ica "
	 * JP: "          Licensed  by          Sony Computer Entertainment Inc.",0x0A
	 *                                                                  |- character we use, at 0x3C
	 */
	switch (data_buffer[0x3C]) {
		case 'E':
			game_region = "European";
			game_is_pal = true;
			break;

		case 'A':
			game_region = "American";
			break;

		case 'I':
			game_region = "Japanese";
			break;

		default:
			game_region = "unknown";
	}

	debug_write("Game's region is %s. Using %s video.", game_region, game_is_pal ? "PAL" : "NTSC");

	// Defaults if no SYSTEM.CNF file exists
	uint32_t tcb = BIOS_DEFAULT_TCB;
	uint32_t event = BIOS_DEFAULT_EVCB;
	uint32_t stacktop = BIOS_DEFAULT_STACKTOP;
	const char * bootfile = "cdrom:PSX.EXE;1";

	char bootfilebuf[32];
	debug_write("Loading SYSTEM.CNF");

	int32_t cnf_fd = FileOpen("cdrom:SYSTEM.CNF;1", FILE_READ);
	if (cnf_fd > 0) {
		read = FileRead(cnf_fd, data_buffer, 2048);
		FileClose(cnf_fd);

		if (read == -1) {
			debug_write("Read error %d", GetLastError());
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
			debug_write("Open error %d", errorCode);
			return;
		}

		debug_write("Not found");
	}

	// Use string format to reduce ROM usage
	debug_write(" * %s = %x", "TCB", tcb);
	debug_write(" * %s = %x", "EVENT", event);
	debug_write(" * %s = %x", "STACK", stacktop);
	debug_write(" * %s = %s", "BOOT", bootfile);

	/*
	 * SetConf is run by BIOS with interrupts disabled.
	 *
	 * If an interrupt happens while the BIOS is reinitializing the TCBs (thread control blocks),
	 * the interrupt handler will store the current thread state in the zero address, wiping
	 * vital data, like the interrupt trampoline at 0x80.
	 *
	 * We do not need to reenable the interrupts because SetConf already does it for us.
	 */
	debug_write("Configuring kernel");
	EnterCriticalSection();
	SetConf(event, tcb, stacktop);

	debug_write("Clearing RAM");
	uint8_t * user_start = (uint8_t *) 0x80010000;
	bzero(user_start, &__RO_START__ - user_start);

	debug_write("Reading executable header");
	int32_t exe_fd = FileOpen(bootfile, FILE_READ);
	if (exe_fd <= 0) {
		debug_write("Open error %d", GetLastError());
		return;
	}
	file_control_block_t * exe_fcb = *BIOS_FCBS + exe_fd;

	read = FileRead(exe_fd, data_buffer, 2048);
	if (read != 2048) {
		debug_write("Missing header. Read %d, error %d.", read, exe_fcb->last_error);
		return;
	}

	exe_header_t * exe_header = (exe_header_t *) data_buffer;
	if (strncmp(exe_header->signature, "PS-X EXE", 8)) {
		debug_write("Header has invalid signature");
		return;
	}

	/*
	 * Patch executable header like stock does. Fixes issue #153 with King's Field (J) (SLPS-00017).
	 * https://github.com/grumpycoders/pcsx-redux/blob/a072e38d78c12a4ce1dadf951d9cdfd7ea59220b/src/mips/openbios/main/main.c#L380-L381
	 */
	exe_header->offsets.initial_sp_base = stacktop;
	exe_header->offsets.initial_sp_offset = 0;

	/*
	 * Patch executable load size, capping it to the file size.
	 *
	 * According to https://github.com/socram8888/tonyhax/issues/161,
	 * Kileak, The Blood (J) (SLPS-00027) specifies in its header a an invalid load size, larger
	 * than the actual executable.
	 *
	 * While the BIOS does not validate this, we do to ensure the file could be read in its
	 * entirety and detect possible CD read errors.
	 */
	uint32_t actual_exe_size = exe_fcb->size - 2048;
	if (actual_exe_size < exe_header->offsets.load_size) {
		exe_header->offsets.load_size = actual_exe_size;
	}

	// If the file overlaps tonyhax, we will use the unstable LoadAndExecute function
	// since that's all we can do.
	if (exe_header->offsets.load_addr + exe_header->offsets.load_size >= &__RO_START__) {
		debug_write("Executable won't fit. Using buggy BIOS call.");

		if (game_is_pal != gpu_is_pal()) {
			debug_write("Switching video mode");
			debug_switch_standard(game_is_pal);
		}

		// Restore original error handler
		bios_restore_disc_error();

		LoadAndExecute(
			bootfile,
			exe_header->offsets.initial_sp_base,
			exe_header->offsets.initial_sp_offset
		);
		return;
	}

	debug_write(
		"Loading executable (%d bytes @ %x)",
		exe_header->offsets.load_size,
		exe_header->offsets.load_addr
	);

	read = FileRead(exe_fd, exe_header->offsets.load_addr, exe_header->offsets.load_size);
	if (read != (int32_t) exe_header->offsets.load_size) {
		debug_write("Failed to load body. Read %d, error %d.", read, exe_fcb->last_error);
		return;
	}

	FileClose(exe_fd);

	patcher_apply(bootfile);

	if (game_is_pal != gpu_is_pal()) {
		debug_write("Switching video mode");
		debug_switch_standard(game_is_pal);
	}

	debug_write("Starting");

	// Restore original error handler
	bios_restore_disc_error();

	// Games from WarmBoot start with interrupts disabled
	EnterCriticalSection();

	// FlushCache needs to be called with interrupts disabled
	FlushCache();

	DoExecute(&exe_header->offsets, 0, 0);
}

void main() {
	// Undo all possible fuckeries during exploiting
	bios_reinitialize();

	// Mute the audio
	audio_halt();

	// Initialize debug screen
	debug_init();

	debug_write("Integrity check %sed", integrity_ok ? "pass" : "fail");
	if (!integrity_ok) {
		return;
	}

	bios_inject_disc_error();
	log_bios_version();

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
#if SOFTUART_PATCH
		patcher_apply_softuart();
		std_out_puts("SoftUART ready\n");
#endif

		try_boot_cd();

		debug_write("Reinitializing kernel");
		bios_reinitialize();
		bios_inject_disc_error();
	}
}

void __attribute__((section(".start"))) start() {
	// Clear BSS
	bzero(&__BSS_START__, &__BSS_END__ - &__BSS_START__);

	// Execute integrity test
	integrity_test();

	main();

	while(1);
}
