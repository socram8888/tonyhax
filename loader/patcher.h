
#pragma once

/**
 * Install and apply suitable BIOS patches.
 */
void patcher_apply(const char * boot_file);

/**
 * Installs the softUART patch.
 */
void patcher_apply_softuart();
