
#include "bios.h"
#include "io.h"
#include "util.h"
#include <string.h>

// Set to zero unless you are using an emulator or have a physical UART on the PS1, else it'll freeze
const uint32_t tty_enabled = 0;

const char * const BIOS_DEVELOPER = (const char *) 0xBFC0012C;
const char * const BIOS_VERSION = (const char *) 0xBFC7FF32;

void bios_reinitialize() {
	// Disable interrupts
	EnterCriticalSection();

	// The following is adapted from the WarmBoot call

	// Mute SPU
	SPU_MAIN_VOL_LEFT = 0;
	SPU_MAIN_VOL_RIGHT = 0;
	SPU_REVERB_VOL_LEFT = 0;
	SPU_REVERB_VOL_RIGHT = 0;

	// Run PS1-specific reset.
	// PS2 don't have them at specific adresses so we'd need to memsearch it.
	if (bios_is_ps1()) {
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
	I_STAT = 0;
	I_MASK = 0;

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

bool bios_is_ps1(void) {
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
	return strncmp(BIOS_DEVELOPER, "CEX-", 4) == 0;
}

bool bios_is_european(void) {
	return BIOS_VERSION[0x20] == 'E';
}
