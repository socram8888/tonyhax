
#include "freepsxpatch.h"
#include "debugscreen.h"
#include "bios.h"

void freepsxpatch_apply(void) {
	// FreePSXBoot is only compatible with PS1, so if it's a PS2 just abort
	if (!bios_is_ps1()) {
		return;
	}

	debug_write("Patching BIOS against FreePSXBoot");
	uint32_t * b_tbl = (uint32_t *) GetB0Table();

	uint32_t read_card_sector = b_tbl[0x4F];
	uint32_t wait_card_status = b_tbl[0x5D];

	// Jal to original function
	b_tbl[0x5E] = 0x23BDFFE8; // subi sp, 0x18
	b_tbl[0x5F] = 0xAFBFFFFC; // sw ra, -4(sp)
	b_tbl[0x60] = 0xAFA40000; // sw a0, 0(sp)
	b_tbl[0x61] = 0xAFA50004; // sw a1, 4(sp)
	b_tbl[0x62] = 0x0C000000 | ((read_card_sector >> 2) & 0x3FFFFFFF); // jal to original call
	b_tbl[0x63] = 0xAFA60008; // sw a2, 8(sp)

	// If zero (failed), abort straight away
	b_tbl[0x64] = 0x10400013; // "beq v0, zero, +19" to word 0x78

	// If sector is not zero, return immediately
	b_tbl[0x65] = 0x8FA50004; // "lw a1, 4(sp)"
	b_tbl[0x66] = 0x14A00011; // "bne a1, zero, +17" to word 0x78

	// If it was, then call wait_card_status
	b_tbl[0x67] = 0x8FA40000; // "lw a0, 0(sp)"
	b_tbl[0x68] = 0x0C000000 | ((wait_card_status >> 2) & 0x3FFFFFFF); // "jal" to wait function
	b_tbl[0x69] = 0x00042102; // "srl a0, 4" to convert port to slot

	// If it did not succeed with 0x01 ready, break returning 1 (as the read call did succeed)
	b_tbl[0x6A] = 0x34080001; // "li t0, 1"
	b_tbl[0x6B] = 0x1448000C; // "bne v0, t0, +12" to word 0x78

	// Check if it contains "FPB" signature, and if so, return 0 so the _bu_init function fails
	b_tbl[0x6C] = 0x8FA60008; // "lw a2, 8(sp)"

	b_tbl[0x6D] = 0x34090046; // "li t1, 0x46"
	b_tbl[0x6E] = 0x90C80010; // "lbu t0, 10(a2)"
	b_tbl[0x6F] = 0x15090008; // "bne t0, t1, +8"

	b_tbl[0x70] = 0x34090050; // "li t1, 0x46"
	b_tbl[0x71] = 0x90C80011; // "lbu t0, 11(a2)"
	b_tbl[0x72] = 0x15090005; // "bne t0, t1, +5"

	b_tbl[0x73] = 0x34090058; // li t1, 0x58
	b_tbl[0x74] = 0x90C80012; // lbu t0, 12(a2)
	b_tbl[0x75] = 0x15090008; // bne t0, t1, +2

	// If matched load zero to v0 to fail and cause _bu_init to ignore us
	b_tbl[0x76] = 0x00000000; // nop
	b_tbl[0x77] = 0x34020000; // li v0, 0

	// Restore registers and return
	b_tbl[0x78] = 0x8FBFFFFC; // "lw ra, -4(sp)"
	b_tbl[0x79] = 0x03E00008; // "jr ra"
	b_tbl[0x7A] = 0x23BD0018; // "addi sp, 0x18"

	// Replace the vector
	b_tbl[0x4F] = (uint32_t) &b_tbl[0x5E];
}
