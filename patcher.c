
#include <stddef.h>
#include <string.h>
#include "crc.h"
#include "debugscreen.h"
#include "patcher.h"

// Last patch in list
#define FLAG_LAST 0x01

// Null the bytes
#define FLAG_NOP  0x02

struct patch {
	uint32_t offset;
	uint16_t size;
	uint8_t flags;
	const uint8_t * data;
};

struct game {
	uint32_t crc;
	const struct patch * patches;
};

const struct game GAMES[] = {
	/*
	 * Tokimeki Memorial 2 (NTSC-J) (SLPM-86355)
	 *
	 * Call is loaded obfuscated from CDPACK00.BIN offset 0x800, and gets loaded to 0x8001000
	 * via CD-ROM DMA (note that write breakpoints don't work in no$psx for DMAs).
	 *
	 * After loading it, the CPU deobfuscates it by adding 0x9C (modulo 256) to every byte. Next,
	 * the antipiracy main function (at 0x8001146C) gets called from 0x80017A24.
	 *
	 * Nuking the call isn't enough as this function sets some global variables:
	 *	*((uint32_t *) 0x8006C69C) = 0x21D; <<< in .data section
	 *	*((uint16_t *) 0x80076BEC) |= 0x8000; <<< in .bss section
	 *  *((uint8_t *)  0x80076BEF) = 1; <<< in .bss section
	 *
	 * Without setting the first the game crashes with an invalid opcode exception, but the later
	 * don't seem to make any difference. Given they are in .bss and they'd get wiped out from
	 * RAM if we set them ourselves, we'll set the first only one and pray the last two aren't
	 * part of some convoluted, Spyro-3-like booby trap.
	 */
	{
		.crc = 0xDEADBEEF,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x80017A24,
				.size = 4,
				.flags = FLAG_NOP
			},
			{
				// Write some variable the game checks.
				// We have to write 0x0000021D, but the last two are already zero.
				.offset = 0x8006C69C,
				.size = 2,
				.flags = FLAG_LAST,
				.data = (const uint8_t[]) { 0x1D, 0x02 }
			}
		}
	},
	/*
	 * Tomba! 2 - The Evil Swine Return (NTSC-U) (SCUS-94454)
	 * Easy to find by looking for reads to address 0xBFC7FF52 (the one the antipiracy uses to
	 * check the BIOS region).
	 */
	{
		.crc = 0xDEADBEEF,
		.patches = (const struct patch[]) {
			{
				// Nuke the call to the check antipiracy function
				.offset = 0x8001127C,
				.size = 4,
				.flags = FLAG_LAST | FLAG_NOP,
			}
		}
	},
	/*
	 * YuGiOh Forbidden Memories (NTSC-U) (SLUS-01411)
	 * This game has the check function in the WA_MRG.MRG file.
	 * It is loaded via CDROM DMA (so don't try to intercept the read via memory write breakpoints)
	 */
	{
		.crc = 0xDEADBEEF,
		.patches = (const struct patch[]) {
			{
				// Nuke the call to the check antipiracy function
				.offset = 0x80043AFC,
				.size = 20,
				.flags = FLAG_LAST | FLAG_NOP,
			}
		}
	},
	/*
	 * YuGiOh Forbidden Memories (PAL-SP) (SLES-03951)
	 */
	{
		.crc = 0xDEADBEEF,
		.patches = (const struct patch[]) {
			{
				// Nuke the call to the check antipiracy function
				.offset = 0x80043F30,
				.size = 20,
				.flags = FLAG_LAST | FLAG_NOP,
			}
		}
	}
};

const struct game * find_game(uint32_t crc) {
	for (uint32_t i = 0; i < sizeof(GAMES) / sizeof(GAMES[0]); i++) {
		if (GAMES[i].crc == crc) {
			return GAMES + i;
		}
	}

	return NULL;
}

void patch_game(const exe_header_t * header) {
	uint32_t exec_crc = crc32(header->load_addr, header->load_size);
	debug_write("Exec CRC: %x", exec_crc);

	const struct game * game = find_game(exec_crc);
	if (game == NULL) {
		return;
	}

	debug_write("Patching:");

	const struct patch * cur_patch = game->patches;
	while (1) {
		debug_write(" - %x", cur_patch->offset);

		uint8_t * patch_dest = (uint8_t *) cur_patch->offset;
		if (cur_patch->flags & FLAG_NOP) {
			memset(patch_dest, 0, cur_patch->size);
		} else {
			memcpy(patch_dest, cur_patch->data, cur_patch->size);
		}

		if (cur_patch->flags & FLAG_LAST) {
			break;
		}

		cur_patch++;
	}
}
