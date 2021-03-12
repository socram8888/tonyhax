
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bios.h"
#include "cdrom.h"
#include "gpu.h"
#include "debugscreen.h"

#define BGCOLOR 0x00C0FF

// Set to zero unless you are using an emulator or have a physical UART on the PS1, else it'll freeze
static const uint32_t tty_enabled = 0;

static uint8_t cd_reply[16];

// Buffer right before this executable
static uint8_t * data_buffer = (uint8_t *) 0x801FB800;

void reinit_kernel() {
	// Disable interrupts
	EnterCriticalSection();

	// The following is adapted from the WarmBoot call

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

bool backdoor_cmd(uint_fast8_t cmd, const char * string) {
	// Send command
	cd_command(cmd, (const uint8_t *) string, strlen(string));

	// Check if INT5, else fail
	uint_fast8_t interrupt = cd_wait_int();
	if (cd_wait_int() != 5) {
		debug_write("Backdoor invalid INT %x", interrupt);
		return false;
	}

	// Check length
	uint_fast8_t reply_len = cd_read_reply(cd_reply);
	if (reply_len != 2) {
		debug_write("Backdoor invalid len = %x", reply_len);
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

	// Run "GetRegion" test
	uint8_t test = CD_TEST_REGION;
	cd_command(CD_CMD_TEST, &test, 1);

	// Should succeed with 3
	if (cd_wait_int() != 3) {
		debug_write("Region read fail");
		return false;
	}

	// Read actual region text
	// No need to bother adding the null terminator as the buffer at this point is all zeros
	cd_read_reply(cd_reply);

	// Compare which is the fifth string we have to send to the backdoor
	const char * region_name;
	const char * p5_localized;
	if (strcmp((char *) cd_reply, "for Europe") == 0) {
		region_name = "European";
		p5_localized = "(Europe)";
	} else if (strcmp((char *) cd_reply, "for U/C") == 0) {
		region_name = "American";
		p5_localized = "of America";
	} else {
		debug_write("Unsupported region");
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
	debug_write("Waiting for lid %s", open ? "open" : "close");

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
	wait_lid_status(true);

	wait_lid_status(false);

	debug_write("Initializing CD");
	CdInit();

	debug_write("Loading SYSTEM.CNF");
	int32_t fd = FileOpen("cdrom:SYSTEM.CNF;1", FILE_READ);
	if (fd == -1) {
		debug_write("Open error");
		return;
	}

	int32_t read = FileRead(fd, data_buffer, 2048);
	FileClose(fd);

	if (read == -1) {
		debug_write("Read error");
		return;
	}

	// Null terminate
	data_buffer[read] = '\0';

	uint32_t tcb, event, stacktop;
	char bootfile[32];
	if (
			!config_get_hex((char *) data_buffer, "TCB", &tcb) ||
			!config_get_hex((char *) data_buffer, "EVENT", &event) ||
			!config_get_hex((char *) data_buffer, "STACK", &stacktop) ||
			!config_get_string((char *) data_buffer, "BOOT", bootfile)
	) {
		return;
	}

	debug_write("Configuring kernel");
	SetConf(event, tcb, stacktop);

	debug_write("Loading executable");
	LoadExeFile(bootfile, data_buffer);

	debug_write("Starting");
	DoExecute(data_buffer, 0, 0);
}

void main() {
	// Turn off screen so the user knows we've successfully started.
	gpu_display(false);

	// Undo all possible fuckeries during exploiting
	reinit_kernel();

	// Initialize debug screen
	debug_init();

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
