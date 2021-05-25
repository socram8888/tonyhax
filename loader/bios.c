
#include "bios.h"
#include "io.h"
#include "util.h"
#include <string.h>

// Set to zero unless you are using an emulator or have a physical UART on the PS1, else it'll freeze
const uint32_t tty_enabled = 0;

// Address of A table
void ** const A0_TBL = (void **) 0x200;

const char * const BIOS_DEVELOPER = (const char *) 0xBFC0012C;
const char * const BIOS_VERSION = (const char *) 0xBFC7FF32;

void bios_reinitialize() {
	// Disable interrupts
	EnterCriticalSection();

	// The following is adapted from the WarmBoot call

	// Copy the relocatable kernel chunk
	bios_copy_relocated_kernel();

	// Reinitialize the A table
	bios_copy_a0_table();

	// Restore A, B and C tables
	init_a0_b0_c0_vectors();

	// Fix A table
	AdjustA0Table();

	// Install default exception handlers
	InstallExceptionHandlers();

	// Restore default exception return function
	SetDefaultExitFromException();

	// Clear interrupts and mask
	I_STAT = 0;
	I_MASK = 0;

	// Setup devices
	InstallDevices(tty_enabled);

	/*
	 * Configure with default values
	 *
	 * SetConf call does:
	 *  - Configure the system memory (via SysInitMemory)
	 *  - Initialize the exception handler arrays
	 *  - Enqueues the syscall handler (via EnqueueSyscallHandler)
	 *  - Initializes the default interrupt (via InitDefInt)
	 *  - Allocates the event handler array
	 *  - Allocates the thread structure
	 *  - Enqueues the timer and VBlank handler (via EnqueueTimerAndVblankIrqs)
	 *  - Calls a function that re-configures the CD subsystem as follows:
	 *      - Enqueues the CD interrupt (via EnqueueCdIntr)
	 *      - Opens shit-ton of CD-related events (via OpenEvent)
	 *      - Enables the CD-related events (via EnableEvent)
	 *      - Re-enables interrupts (via ExitCriticalSection)
	 *
	 * This call is to be used after the CdInit has happened, once we've read the SYSTEM.CNF
	 * file and we want to reconfigure the kernel before launching the game's executable.
	 *
	 * However, for the purpose of reinitializing the BIOS, the CD reinitialization procedure is
	 * problematic, as CdInit (which we'll call later once the disc is swapped) calls this very
	 * same function, resulting in the CD interrupt being added twice to the array of handlers,
	 * as wells as events being opened twice.
	 *
	 * We can't patch this code because it's stored in ROM. Instead, before calling this function
	 * we'll replace the EnqueueCdIntr with a fake version that patches the system state to return
	 * earlier and avoid the CD reinitialization entirely.
	 */
	void * realEnqueueCdIntr = A0_TBL[0xA2];
	A0_TBL[0xA2] = FakeEnqueueCdIntr;
	SetConf(BIOS_DEFAULT_EVCB, BIOS_DEFAULT_TCB, BIOS_DEFAULT_STACKTOP);
	A0_TBL[0xA2] = realEnqueueCdIntr;

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

void * parse_warmboot_jal(uint32_t opcode) {
	const uint32_t * warmboot_start = (const uint32_t *) A0_TBL[0xA0];
	uint32_t prefix = (uint32_t) warmboot_start & 0xF0000000;

	uint32_t * jal = (uint32_t *) (warmboot_start + opcode);
	uint32_t suffix = (*jal & 0x3FFFFFF) << 2;

	return (void *) (prefix | suffix);
}

void bios_copy_relocated_kernel(void) {
	/*
	 * This function indirectly calls the BIOS function that copies the relocated kernel code to
	 * 0x500.
	 *
	 * The 12th opcode of WarmBoot is a "jal" to this function for all BIOS I've checked,
	 * including the PS2 consoles in PS1 mode.
	 */

	void * address = parse_warmboot_jal(12);
	((void (*)(void)) address)();
}

void bios_copy_a0_table(void) {
	/*
	 * This function indirectly calls the BIOS function that copies the A0 table to 0x200.
	 *
	 * As with the kernel relocation function, the 14th opcode of WarmBoot is a "jal" to this
	 * function.
	 */

	void * address = parse_warmboot_jal(14);
	((void (*)(void)) address)();
}

handler_info_t * bios_get_syscall_handler(void) {
	/*
	 * This function extracts the pointer to the syscall handler by analyzing the opcodes of the
	 * EnqueueSyscallHandler function.
	 */
	uint32_t * func_start = (uint32_t *) GetC0Table()[1];

	// The fourth instruction is a one opcode "la a1, ADDR". Extract it from there.
	return (handler_info_t *) (func_start[4] & 0xFFFF);
}
