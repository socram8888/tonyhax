
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "bios.h"
#include "cdrom.h"
#include "gpu.h"
#include "debugscreen.h"
#include "hash.h"

// Set to zero unless you are using an emulator or have a physical UART on the PS1, else it'll freeze
const uint32_t tty_enabled = 0;

// Loading address of tonyhax, provided by the secondary.ld linker script
extern uint8_t __ROM_START__, __ROM_END__;

// Buffer right before this executable
uint8_t * const data_buffer = (uint8_t *) 0x801FB800;

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
	 * The integrity consists of cdb hash inside the title frame after the title name,
	 * at offset 0x4C.
	 * This hash is calculated over all the read-only payload.
	 */
	uint32_t correct_value = *((uint32_t * ) (&__ROM_START__ - 0x100 + 0x4C));
	uint32_t calc_value = cdb_hash(&__ROM_START__, &__ROM_END__ - &__ROM_START__);

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
	uint_fast8_t interrupt = cd_wait_int();
	if (cd_wait_int() != 5) {
		debug_write("Bdoor invalid INT %x", interrupt);
		return false;
	}

	// Check length
	uint_fast8_t reply_len = cd_read_reply(cd_reply);
	if (reply_len != 2) {
		debug_write("Bdoor invalid len %x", reply_len);
		return false;
	}

	// Check there is an error flagged
	if (!(cd_reply[0] & 0x01)) {
		debug_write("Backdoor reply invalid");
		return false;
	}

	// Check error code
	if (cd_reply[1] != 0x40) {
		debug_write("Backdoor reply invalid");
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
		debug_write("Region read fail");
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
	} else if (strcmp((char *) cd_reply, "for US/AEP") == 0) { /* DTL-H1202 */
		region_name = "Debug 2.2 (EU)";
		p5_localized = "DONTCARE";
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

bool config_get_hex(const char * config, const char * wanted, uint32_t * value) {
	uint32_t wanted_len = strlen(wanted);

	while (true) {
		// Check if first N characters match
		if (strncmp(config, wanted, wanted_len) == 0) {
			// Perfect, we're on the right line. Advance.
			config += wanted_len;

			// Keep parsing until we hit the end of line or the end of file
			uint32_t parsed = 0;
			while (*config != '\n' && *config != '\0') {
				uint32_t digit = todigit(*config);
				if (digit < 0x10) {
					parsed = parsed << 4 | digit;
				}
				config++;
			}

			// Log
			debug_write("%s = %x", wanted, parsed);
			*value = parsed;
			return true;

		} else {
			// No luck. Advance until next line.
			config = strchr(config, '\n');
			
			// If this is the last line, abort.
			if (config == NULL) {
				debug_write("Missing %s", wanted);
				return false;
			}

			// Advance to skip line feed.
			config++;
		}
	}
}

bool config_get_string(const char * config, const char * wanted, char * value) {
	uint32_t wanted_len = strlen(wanted);

	while (true) {
		// Check if first N characters match
		if (strncmp(config, wanted, wanted_len) == 0) {
			// Perfect, we're on the right line. Advance.
			config += wanted_len;

			// Advance until the start
			while (1) {
				// Skip spaces and equals
				if (*config == ' ' || *config == '=') {
					config++;
				} else if (*config == '\0' || *config == '\n' || *config == '\r') {
					debug_write("Corrupted %s", wanted);
					return false;
				} else {
					break;
				}
			}

			// Copy until space or end of file
			char * valuecur = value;
			while (*config != '\0' && *config != '\n' && *config != '\r' && *config != ' ') {
				*valuecur = *config;
				config++;
				valuecur++;
			}

			// Null terminate
			*valuecur = '\0';

			// Log
			debug_write("%s = %s", wanted, value);
			return true;

		} else {
			// No luck. Advance until next line.
			config = strchr(config, '\n');
			
			// If this is the last line, abort.
			if (config == NULL) {
				debug_write("Missing %s", wanted);
				return false;
			}

			// Advance to skip line feed.
			config++;
		}
	}
}

void try_boot_cd() {
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
	int32_t cfg_fd = FileOpen("cdrom:SYSTEM.CNF;1", FILE_READ);
	if (cfg_fd > 0) {
		int32_t read = FileRead(cfg_fd, data_buffer, 2048);
		FileClose(cfg_fd);

		if (read == -1) {
			debug_write("Read error %x", GetLastError());
			return;
		}

		// Null terminate
		data_buffer[read] = '\0';

		if (
				!config_get_hex((char *) data_buffer, "TCB", &tcb) ||
				!config_get_hex((char *) data_buffer, "EVENT", &event) ||
				!config_get_hex((char *) data_buffer, "STACK", &stacktop) ||
				!config_get_string((char *) data_buffer, "BOOT", bootfilebuf)
		) {
			return;
		}

		bootfile = bootfilebuf;
	} else {
		uint32_t errorCode = GetLastError();
		if (errorCode != FILEERR_NOT_FOUND) {
			debug_write("Open error %x", errorCode);
			return;
		}

		debug_write("Not found. Using PSX.EXE");
	}

	debug_write("Configuring kernel");
	SetConf(event, tcb, stacktop);

	debug_write("Loading executable");
	if (!LoadExeFile(bootfile, data_buffer)) {
		debug_write("Loading failed");
		return;
	}

	debug_write("Starting");
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

	debug_write("Unlocking CD drive");

	if (!unlock_drive()) {
		return;
	}

	debug_write("Unlocked successfully!");

	while (1) {
		try_boot_cd();
	}
}

void __attribute__((section(".start"))) start() {
	main();
	while(1);
}
