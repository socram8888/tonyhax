
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
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

// Buffer right before this executable
uint8_t * const data_buffer = (uint8_t *) 0x801F9800;

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
	CdInit();

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
	debug_write(" * %s = %x", "TCB", tcb);
	debug_write(" * %s = %x", "EVENT", event);
	debug_write(" * %s = %x", "STACK", stacktop);
	debug_write(" * %s = %s", "BOOT", bootfile);

	debug_write("Configuring kernel");
	SetConf(event, tcb, stacktop);

	debug_write("Clearing RAM");
	uint8_t * user_start = (uint8_t *) 0x80010000;
	bzero(user_start, &__RO_START__ - user_start);

	debug_write("Checking executable");
	int32_t exe_fd = FileOpen(bootfile, FILE_READ);
	if (exe_fd <= 0) {
		debug_write("Open error %x", GetLastError());
		return;
	}

	read = FileRead(exe_fd, data_buffer, 2048);

	if (read != 2048) {
		debug_write("Read error %x", GetLastError());
		return;
	}

	exe_header_t * exe_header = (exe_header_t *) (data_buffer + 0x10);

	// If the file overlaps tonyhax, we will use the unstable LoadAndExecute function
	// since that's all we can do.
	if (exe_header->load_addr + exe_header->load_size >= data_buffer) {
		debug_write("Won't fit. Using BIOS.");

		if (game_is_pal != gpu_is_pal()) {
			debug_write("Switching video mode");
			debug_switch_standard(game_is_pal);
		}

		LoadAndExecute(bootfile, exe_header->initial_sp_base, exe_header->initial_sp_offset);
		return;
	}

	debug_write("Loading executable (%x @ %x)", exe_header->load_size, exe_header->load_addr);

	if (FileRead(exe_fd, exe_header->load_addr, exe_header->load_size) != (int32_t) exe_header->load_size) {
		debug_write("Read error %x", GetLastError());
		return;
	}

	FileClose(exe_fd);

	if (game_is_pal != gpu_is_pal()) {
		debug_write("Switching video mode");
		debug_switch_standard(game_is_pal);
	}

	debug_write("Starting");

	// Games from WarmBoot start with interrupts disabled
	EnterCriticalSection();

	// FlushCache needs to be called with interrupts disabled
	FlushCache();

	DoExecute(exe_header, 0, 0);
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
		patcher_apply();
		try_boot_cd();

		debug_write("Reinitializing kernel");
		bios_reinitialize();
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
