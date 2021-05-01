
#pragma once

/**
 * Applies a patch to prevent the game from using a game card with FreePSXBoot installed,
 * so the user does not need to remove the card physically from the PS1.
 *
 * The way this is done is by hijacking the "read_card_sector" function (table A, function 0x4F).
 *
 * If the sector being read is sector 0 and it contains "FPBZ" at +0x7C, we modify the read data
 * so it is detected as corrupted and the game skips reading from it.
 *
 * As FreePSXBoot is only compatible with PS1, this function does nothing on a PS2.
 */
void freepsxpatch_apply(void);
