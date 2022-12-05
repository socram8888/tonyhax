
#include "bios.h"
#include "debugscreen.h"
#include "str.h"
#include "patcher.h"

inline void encode_j(void * jump_location, const void * jump_dest) {
	uint32_t * words = (uint32_t *) jump_location;
	words[0] = 0x08000000 | (((uint32_t) jump_dest >> 2) & 0x3FFFFFF);
}

inline void encode_jal(void * jump_location, const void * jump_dest) {
	uint32_t * words = (uint32_t *) jump_location;
	words[0] = 0x0C000000 | (((uint32_t) jump_dest >> 2) & 0x3FFFFFF);
}

inline void encode_li(void * load_location, int regnum, uint32_t value) {
	uint32_t * words = (uint32_t *) load_location;

	// LUI - Load Upper Immediate
	words[0] = 0x3C000000 | (regnum << 16) | (value >> 16);

	// ORI - OR Immediate
	words[1] = 0x34000000 | (regnum << 21) | (regnum << 16) | (value & 0xFFFF);
}

uint8_t * install_generic_antipiracy_patch(uint8_t * install_addr) {
	// Exports defined by the patch
	extern uint8_t patch_ap_start;
	extern uint8_t patch_ap_end;
	extern uint8_t patch_ap_skip;
	extern uint8_t patch_ap_success;

	debug_write(" * Generic antipiracy");

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
	encode_j(install_addr + (&patch_ap_skip - &patch_ap_start), cases_array[1]);

	/*
	 * Insert the jump we'll use to exit the exception handler once we have finished patching up
	 * the thread state if the call was indeed originated from an antipiracy module.
	 *
	 * We'll use the address of syscall(0) which behaves as a nop to exit the exception.
	 */
	encode_j(install_addr + (&patch_ap_success - &patch_ap_start), cases_array[0]);

	// Finally replace
	cases_array[1] = install_addr;

	return install_addr + (&patch_ap_end - &patch_ap_start);
}

uint8_t * install_vandal_patch(uint8_t * install_addr) {
	// Exports defined by the patch
	extern uint8_t patch_vandal_start;
	extern uint8_t patch_vandal_return;
	extern uint8_t patch_vandal_end;

	debug_write(" * Vandal Hearths 2 AP");

	// Copy blob
	memcpy(install_addr, &patch_vandal_start, &patch_vandal_end - &patch_vandal_start);

	// Hook into call 16 of table B (OutdatedPadGetButtons), which is called once per frame
	void ** b0_tbl = GetB0Table();

	// Insert call to real function
	encode_j(install_addr + (&patch_vandal_return - &patch_vandal_start), b0_tbl[0x16]);

	// Replace it now
	b0_tbl[0x16] = install_addr;

	// Advance installation address
	return install_addr + (&patch_vandal_end - &patch_vandal_start);
}

uint8_t * install_fpb_patch(uint8_t * install_addr) {
	// Exports defined by the patch
	extern uint8_t patch_fpb_start;
	extern uint8_t patch_fpb_end;

	debug_write(" * FreePSXBoot");

	// Copy blob
	memcpy(install_addr, &patch_fpb_start, &patch_fpb_end - &patch_fpb_start);

	// Install it
	encode_jal((void *) 0x5B40, install_addr);

	// Advance installation address
	return install_addr + (&patch_fpb_end - &patch_fpb_start);
}

void patcher_apply(const char * boot_file) {
	// We have plenty of space at the end of table B
	uint8_t * install_addr = (uint8_t *) (GetB0Table() + 0x5E);

	// Install patches
	debug_write("Installing patches:");

	// Install a suitable antimodchip patch
	if (strcmp(boot_file, "cdrom:\\SLUS_009.40;1") == 0) {
		install_addr = install_vandal_patch(install_addr);
	} else {
		install_addr = install_generic_antipiracy_patch(install_addr);
	}

	// FreePSXBoot does not work on PS2 so skip its installation
	if (bios_is_ps1()) {
		install_addr = install_fpb_patch(install_addr);
	}
}

void patcher_apply_softuart() {
	// Exports defined by the patch
	extern uint8_t patch_uartputc_start;
	extern uint8_t patch_uartputc_end;

	// Overwrite BIOS' std_out_putchar function
	memcpy(BIOS_A0_TABLE[0x3C], &patch_uartputc_start, &patch_uartputc_end - &patch_uartputc_start);
}
