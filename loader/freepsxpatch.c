
#include "freepsxpatch.h"
#include "debugscreen.h"
#include "bios.h"
#include <string.h>

/*
 * The patch itself.
 *
 * It will be copied to 0xA00, which is empty (null) space in the B table. This was never used by
 * any official BIOS patches so we should be safe.
 *
 * This patch is called right at the very end of the last step in the read sector finite state
 * machine:
 * https://github.com/grumpycoders/pcsx-redux/blob/f6484e8010a40a81e4019d9bfa1a9d408637b614/src/mips/openbios/sio0/card.c#L194
 *
 * When this code is executed, the registers are as follows:
 *   - v0 contains 1, or "success".
 *   - a1 contains the read buffer
 *   - a2 contains the current sector number
 *
 * The offsets have been checked against BIOSes 2.2, 3.0, 4.1 and 4.4
 */
static const uint32_t patch_code[] = {
	0x14C00006, // "bne a2, 0, +6" to the jump to return
	0x8CA8007C, // "lw t0, 0x7C(a1)"
	0x3C095A42, // "lui t1, 0x5A42"
	0x35295046, // "ori t1, t1, 0x5046"
	0x15090002, // "bne t0, t1, +2"
	0x00000000, // "nop"
	0xACA00000, // "sw 0, 0(a1)"
	0x080016D5, // "j 0x5B54" to return to normal flow
};

void freepsxpatch_apply(void) {
	// FreePSXBoot is only compatible with PS1, so if it's a PS2 just abort
	if (!bios_is_ps1()) {
		return;
	}

	debug_write("Patching BIOS against FreePSXBoot");

	// Copy patch code
	uint32_t * patch_location = (uint32_t *) 0xA00;
	memcpy(patch_location, patch_code, sizeof(patch_code));

	// Insert jump to the patch
	uint32_t * entry_jal = (uint32_t *) 0x5B40;
	*entry_jal = 0x08000280; // "j 0xA00"
}
