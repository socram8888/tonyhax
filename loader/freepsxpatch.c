
#include "freepsxpatch.h"
#include "debugscreen.h"
#include "bios.h"

void freepsxpatch_apply(void) {
	// FreePSXBoot is only compatible with PS1, so if it's a PS2 just abort
	if (!bios_is_ps1()) {
		return;
	}

	debug_write("Patching BIOS against FreePSXBoot");

	// We'll hijack this empty space to store the check for the "FPXB" marker on the MC's first sector
	uint32_t * b_tbl = (uint32_t *) GetB0Table();

	// Insert the payload
	// At this point the sector number is in a2, read buffer is in a1 and v1 is already 1 (success)
	b_tbl[0x5E] = 0x14C00006; // "bne a2, 0, +6" to the jump to return
	b_tbl[0x5F] = 0x8CA80010; // "lw t0, 10(a1)"
	b_tbl[0x60] = 0x3C094258; // "lui t1, 0x4258"
	b_tbl[0x61] = 0x35295046; // "ori t1, t1, 0x5046"
	b_tbl[0x62] = 0x15090002; // "bne t0, t1, +2"
	b_tbl[0x63] = 0x00000000; // "nop"
	b_tbl[0x64] = 0x2402FFFF; // "li v0, -1"
	b_tbl[0x65] = 0x080016D5; // "j 0x5B54" to return to normal flow

	// Offset from a 4.4 BIOS. Will need to confirm if this is/isn't constant
	// Final clause of the read sector FSM:
	// https://github.com/grumpycoders/pcsx-redux/blob/f6484e8010a40a81e4019d9bfa1a9d408637b614/src/mips/openbios/sio0/card.c#L194
	uint32_t * entry_jal = (uint32_t *) 0x5B40;

	// Replace with a jump to the patcher
	*entry_jal = 0x08000000 | (((uint32_t) &b_tbl[0x5E] >> 2) & 0x3FFFFFFF);
}
