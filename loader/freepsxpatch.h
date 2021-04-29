
#pragma once

/**
 * Applies a patch to prevent the game from using a game card with FreePSXBoot installed,
 * so the user does not need to remove the card physically from the PS1.
 *
 * The way this is done is by hijacking the "read_card_sector" function (table A, function 0x4F).
 *
 * If the sector being read is sector 0 and it contains "FPSX" on +0x10, we'll return a "memory
 * card not found" error (0x11).
 *
 * This *might* cause issues as we will be executing the read for that particular sector
 * synchronously, locking until it finishes which is not what the BIOS is supposed to do.
 *
 * This patch will be stored from words 0x5E to 0xFF of the B table itself, which are null and are
 * never used by any official BIOS patch that could collide with us.
 */
void freepsxpatch_apply(void);
