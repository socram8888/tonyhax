
#include "bios.h"
#include "debugscreen.h"
#include <string.h>
#include "patcher.h"

#include "patches.inc"

inline uint32_t encode_j(const void * addr) {
	return 0x08000000 | (((uint32_t) addr >> 2) & 0x3FFFFFF);
}

inline uint32_t encode_jal(const void * addr) {
	return 0x0C000000 | (((uint32_t) addr >> 2) & 0x3FFFFFF);
}

void install_modchip_patch(uint8_t * patches_addr) {
	debug_write("Installing modchip patch");

	// Get the handler info structure
	handler_info_t * syscall_handler = bios_get_syscall_handler();

	// Get the start of the verifier function (the only one set)
	uint32_t * verifier = (uint32_t *) syscall_handler->verifier;

	/*
	 * At opcode 20 it accesses an 4-word array which contain where to jump depending on the
	 * syscall performed. We're interested in modifying the value for 1 (EnterCriticalSection)
	 * so we can intercept it and defuse the antimodchip.
	 */
	uint32_t lw_op = verifier[20];
	if ((lw_op >> 16) != 0x8C39) {
		debug_write("Check failed!");
		return;
	}
	void ** cases_array = (void **) (lw_op & 0xFFFF);

	/*
	 * Insert the jump to the original code, which we'll use if the call was not originated from
	 * an antipiracy module.
	 */
	*((uint32_t *) (patches_addr + BIOS_PATCHES_MODCHIPCONTINUE)) = encode_j(cases_array[1]);

	/*
	 * Insert the jump we'll use to exit the exception handler once we have finished patching up
	 * the thread state if the call was indeed originated from an antipiracy module.
	 *
	 * We'll use the address of syscall(0) which behaves as a nop to exit the exception.
	 */
	*((uint32_t *) (patches_addr + BIOS_PATCHES_MODCHIPRETURN)) = encode_j(cases_array[0]);

	// Finally replace
	cases_array[1] = patches_addr + BIOS_PATCHES_MODCHIPSTART;
}

void install_fpx_patch(uint8_t * patches_addr) {
	if (!bios_is_ps1()) {
		return;
	}

	debug_write("Installing FreePSXBoot patch");
	*((uint32_t *) 0x5B40) = encode_jal(patches_addr + BIOS_PATCHES_ANTIFPXPATCH);
}

void patcher_apply(void) {
	// We have plenty of space at the end of table B
	uint8_t * start_addr = (uint8_t *) (GetB0Table() + 0x5E);

	// Copy patches
	debug_write("Copying patches to %x", (uint32_t) start_addr);
	memcpy(start_addr, BIOS_PATCHES_BLOB, sizeof(BIOS_PATCHES_BLOB));

	// Install patches
	install_modchip_patch(start_addr);
	install_fpx_patch(start_addr);
}
