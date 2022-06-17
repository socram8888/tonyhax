
#include "cdrom.h"
#include "bios.h"
#include <stddef.h>

volatile uint8_t * const CD_REGS = (volatile uint8_t *) 0x1F801800;

inline void cd_set_page(uint8_t page) {
	CD_REGS[0] = page;
}

void cd_command(uint_fast8_t cmd, const uint8_t * params, uint_fast8_t params_len) {

	// Wait for previous command to finish, if any
	while (CD_REGS[0] & 0x80);

	// Switch to page 0
	cd_set_page(0);

	// Clear read and write FIFOs
	CD_REGS[3] = 0xC0;

	// Copy request
	while (params_len != 0) {
		CD_REGS[2] = *params;
		params++;
		params_len--;
	}

	// Switch to page 1
	cd_set_page(1);

	// Disable interrupts as we'll poll
	CD_REGS[2] = 0x00;

	// Acknowledge interrupts, if there were any
	CD_REGS[3] = 0x07;

	// Switch to page 0
	cd_set_page(0);

	// Finally write command to start
	CD_REGS[1] = cmd;
}

uint_fast8_t cd_wait_int(void) {

	// Wait for command to finish, if any
	while (CD_REGS[0] & 0x80);

	// Switch to page 1
	cd_set_page(1);

	// Wait until an interrupt happens (int != 0)
	uint_fast8_t interrupt;
	do {
		interrupt = CD_REGS[3] & 0x07;
	} while (interrupt == 0);

	// Acknowledge it
	CD_REGS[3] = 0x07;

	// Return it
	return interrupt;
}

uint_fast8_t cd_read_reply(uint8_t * reply_buffer) {

	// Switch to page 1
	cd_set_page(1);

	// Read reply
	uint_fast8_t len = 0;
	while (CD_REGS[0] & 0x20) {
		*reply_buffer = CD_REGS[1];
		reply_buffer++;
		len++;
	}

	// Return length
	return len;
}

bool cd_drive_init() {
	cd_command(CD_CMD_INIT, NULL, 0);

	// Should succeed with 3
	if (cd_wait_int() != 3) {
		return false;
	}

	// Should then return a 2
	if (cd_wait_int() != 2) {
		return false;
	}

	return true;
}

void cd_drive_reset() {
	// Issue a reset (looses authentication and or unlock so do an unlock after this)
	cd_command(CD_CMD_RESET, NULL, 0);

	// Should succeed with 3 but doesn't sometimes so we can't check the return value
	cd_wait_int();

	// Need to wait for some cycles before it springs back to life
	for (int i = 0; i < 0x400000; i++);
}
