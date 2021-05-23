
#include "bios.h"
#include "debugscreen.h"
#include <string.h>
#include "patcher.h"

#include "patches.inc"

/*
 * 0xA00 is empty (null) space in the B table. This was never used by any official BIOS patches
 * so we should be safe.
 */
static uint8_t * const PATCH_START_ADDR = (uint8_t *) 0xA00;

inline uint32_t encode_jal(const void * addr) {
	return 0x08000000 | (((uint32_t) addr >> 4) & 0x3FFFFFF);
}

void install_modchip_patch() {
	debug_write("Installing modchip patch (TODO!)");
}

void install_fpx_patch() {
	if (bios_is_ps1()) {
		return;
	}

	debug_write("Installing FreePSXBoot patch");
	*((uint32_t *) 0x5B40) = encode_jal(PATCH_START_ADDR + BIOS_PATCHES_ANTIFPXPATCH);
}

void patcher_apply(void) {
	// Copy patches
	memcpy(PATCH_START_ADDR, BIOS_PATCHES_BLOB, sizeof(BIOS_PATCHES_BLOB));

	// Install patches
	install_modchip_patch();
	install_fpx_patch();
}
