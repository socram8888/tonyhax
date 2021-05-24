
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

// Let's just pray these don't vary
void ** const HANDLER_ARRAY = (void **) 0x100;
uint32_t * const HANDLER_ARRAY_LEN = (uint32_t *) 0x104;

struct boot_cnf_t {
	uint32_t event;
	uint32_t tcb;
	uint32_t stacktop;
};

void initHandlersArray(int32_t priorities) {
	// 8 is the size of the struct, whose contents we don't really care about
	uint32_t array_len = 8 * priorities;

	void * array = alloc_kernel_memory(array_len);
	if (array) {
		bzero(array, array_len);

		*HANDLER_ARRAY = array;
		*HANDLER_ARRAY_LEN = array_len;
	}
}

void bios_reinitialize() {
	// Disable interrupts
	EnterCriticalSection();

	// The following is adapted from the WarmBoot call

	// Mute SPU
	SPU_MAIN_VOL_LEFT = 0;
	SPU_MAIN_VOL_RIGHT = 0;
	SPU_REVERB_VOL_LEFT = 0;
	SPU_REVERB_VOL_RIGHT = 0;

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
	 * We can't use SetConf as that one re-queues the CD events (and that's no bueno as we
	 * haven't initialized the system's memory yet).
	 */
	boot_cnf_t * boot_cnf = bios_get_config();
	boot_cnf->event = 4;
	boot_cnf->tcb = 16;
	boot_cnf->stacktop = 0x801FFF00;

	// Initialize kernel memory
	SysInitMemory(0xA000E000, 0x2000);

	// Initialize handlers array
	initHandlersArray(4);

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

boot_cnf_t * bios_get_config() {
	/*
	 * Extract from the "la t6, boot_cnf_values+8" opcodes at the start of the GetConf function.
	 *
	 * According to psx-spx docs this was used by Spec Ops Airborne Commando so it should work
	 * on every retail BIOS.
	 */
	const uint8_t * get_conf_offset = (const uint8_t *) A0_TBL[0x9D];

	// Extract high from the "lui"
	uint16_t high = *((uint16_t *) (get_conf_offset + 0x00));

	// Extract low from the "addi"
	int16_t low = *((int16_t *) (get_conf_offset + 0x04));

	return (boot_cnf_t *) (((int32_t) high << 16) + low - 8);
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
