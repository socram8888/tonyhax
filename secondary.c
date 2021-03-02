
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bios.h"
#include "cdrom.h"

// Set to zero unless you are using an emulator or have a physical UART on the PS1, else it'll freeze
static const uint32_t tty_enabled = 1;

static uint8_t cd_reply[16];

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
	cd_command(cmd, string, strlen(string));

	// Check if INT5, else fail
	if (cd_wait_int() != 5) {
		std_out_puts("Invalid INT\n");
		return false;
	}

	// Check length
	if (cd_read_reply(&cd_reply) != 2) {
		std_out_puts("Invalid reply\n");
		return false;
	}

	// Check there is an error flagged
	if (!(cd_reply[0] & 0x01)) {
		std_out_puts("Invalid reply\n");
		return false;
	}

	// Check error code
	if (cd_reply[1] != 0x40) {
		std_out_puts("Invalid reply\n");
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
		std_out_puts("Failed to read region\n");
		return false;
	}

	// Read actual region text
	// No need to bother adding the null terminator as the buffer at this point is all zeros
	int replylen = cd_read_reply(&cd_reply);

	std_out_puts("Region: \"");
	// +4 to skip the "for " at the beginning of the reply
	std_out_puts((char *) (cd_reply + 4));
	std_out_puts("\"\n");

	// Compare which is the fifth string we have to send to the backdoor
	const char * p5_localized;
	if (strcmp((char *) cd_reply, "for Europe") == 0) {
		p5_localized = "(Europe)";
	} else if (strcmp((char *) cd_reply, "for U/C") == 0) {
		p5_localized = "of America";
	} else {
		std_out_puts("Unsupported region\n");
		return false;
	}

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
		std_out_puts("Backdoor failed\n");
		return false;
	}

	return true;
}

void main() {
	// Tell the user we've successfully launched
	std_out_puts("=== SECONDARY PROGRAM LOADER ===\n");

	std_out_puts("Reinitializing kernel... ");

	// Undo all possible fuckeries during exploiting
	reinit_kernel();

	std_out_puts("success\n");

	std_out_puts("Unlocking drive...\n");

	if (!unlock_drive()) {
		return;
	}

	std_out_puts("Unlocked!\n");
}

void __attribute__((section(".start"))) start() {
	main();
	while(1);
}
