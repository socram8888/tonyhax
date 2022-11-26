
#include "bios.h"
#include "debugscreen.h"
#include "str.h"
#include "patcher.h"

extern uint8_t patch_ap_start;
extern uint8_t patch_ap_end;
extern uint8_t patch_ap_skip;
extern uint8_t patch_ap_success;

extern uint8_t patch_fpb_start;
extern uint8_t patch_fpb_end;

inline uint32_t encode_j(const void * addr) {
	return 0x08000000 | (((uint32_t) addr >> 2) & 0x3FFFFFF);
}

inline uint32_t encode_jal(const void * addr) {
	return 0x0C000000 | (((uint32_t) addr >> 2) & 0x3FFFFFF);
}

uint8_t * install_generic_modchip_patch(uint8_t * install_addr) {
	debug_write(" * Generic antimodchip");

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
		debug_write("Aborted! Please report this!");
		return install_addr;
	}

	// Extract location of cases array
	void ** cases_array = (void **) (lw_op & 0xFFFF);

	// Copy blob
	memcpy(install_addr, &patch_ap_start, &patch_ap_end - &patch_ap_start);

	/*
	 * Insert the jump to the original code, which we'll use if the call was not originated from
	 * an antipiracy module.
	 */
	*((uint32_t *) (install_addr + (&patch_ap_skip - &patch_ap_start))) = encode_j(cases_array[1]);

	/*
	 * Insert the jump we'll use to exit the exception handler once we have finished patching up
	 * the thread state if the call was indeed originated from an antipiracy module.
	 *
	 * We'll use the address of syscall(0) which behaves as a nop to exit the exception.
	 */
	*((uint32_t *) (install_addr + (&patch_ap_success - &patch_ap_start))) = encode_j(cases_array[0]);

	// Finally replace
	cases_array[1] = install_addr;

	return install_addr + (&patch_ap_end - &patch_ap_start);
}

uint8_t * install_fpb_patch(uint8_t * install_addr) {
	debug_write(" * FreePSXBoot");

	// Copy blob
	memcpy(install_addr, &patch_fpb_start, &patch_fpb_end - &patch_fpb_start);

	// Install it
	*((uint32_t *) 0x5B40) = encode_jal(install_addr);

	// Advance installation address
	return install_addr + (&patch_fpb_end - &patch_fpb_start);
}

void patcher_apply(void) {
	// We have plenty of space at the end of table B
	uint8_t * install_addr = (uint8_t *) (GetB0Table() + 0x5E);

	// Install patches
	debug_write("Installing patches:");

	// Generic antimodchip, which works on pretty much every game with antipiracy
	install_addr = install_generic_modchip_patch(install_addr);

	// FreePSXBoot does not work on PS2 so skip its installation
	if (bios_is_ps1()) {
		install_addr = install_fpb_patch(install_addr);
	}
}
