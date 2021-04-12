
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
	 * Aconcagua (J) (Disc 1) (SCPS-10131)
	 *
	 * Another though one.
	 *
	 * This one uses the main executable just as a loader for other dynamically-loaded executables,
	 * that are loaded from PROGRAM.BIN.
	 *
	 * This is done using a function at 0x80011DE4, which takes in a0 the sector relative to the
	 * beginning of the PROGRAM.BIN file where to load the executable from (one sector = 2048
	 * bytes)
	 *
	 * Loading uses a complex function at 0x80012108, which is called from 0x80011E2C. This
	 * loads from the disc the executable and decompresses it.
	 *
	 * Executing is just a wrapper around the BIOS DoExecute method, resides at 0x80011E34, and is
	 * called from 0x80011E34.
	 *
	 * Now the details on how the patching works:
	 *
	 *  - First, we'll hijack the call at 0x80011E0C (originally "jal 0x80016BA0") so we can save
	 *    the sector from where the exe is being loaded from, which at that point resides in
	 *    register s0. This shim will save it into a fixed address into RAM, and then continue
	 *    the normal execution.
	 *
	 *  - Second, we'll hijack the call to the DoExecute wrapper at 0x80011E34 (originally
	 *    "jal 0x80011E74"). This second shim will check if the loaded executable is the first
	 *    one (the one that contains the antipiracy, with sector = 0), and if so, will patch the
	 *    contents. It will then go back to the normal flow.
	 *
	 * These shims and sector will be stored at 0x80010D08, which contains just a debug text.
	 */
	{
		.crc = 0x4828F3AD,
		.patches = (const struct patch[]) {
			{
				// Shims
				.offset = 0x80010D0C,
				.size = 32,
				.data = (const uint8_t[]) {
					// First shim, at 0x80010D0C
					0xE8, 0x5A, 0x00, 0x08, // "j 0x80016BA0" to continue with the normal execution flow
					0xF4, 0xEE, 0xF0, 0xAF, // "sw s0, -0x110C(ra)" where -0x110C is the difference between 0x80010D08 and ra (0x80011E14)

					// Second shim, at 0x80010D14
					0xCC, 0xEE, 0xE8, 0x8F, // "lw t0, -0x1134(ra)" where -0x1134 is the difference between 0x80010D08 and ra (0x80011E3C)
					0x02, 0x00, 0x00, 0x15, // "bnez t0, 0x80010D24"
					0x02, 0x80, 0x01, 0x3C, // "lui at, 0x8002"
					0x94, 0x68, 0x20, 0xAC, // "sw 0, 0x6894(at)" to nuke the call to antipiracy at 0x80026894
					0x9D, 0x47, 0x00, 0x08, // "j 0x80011E74" to continue with the normal execution flow
					0x00, 0x00, 0x00, 0x00  // "nop"
				}
			},
			{
				// Hijack the first call to save the sector
				.offset = 0x80011E0C,
				.size = 4,
				.data = (const uint8_t[]) {
					0x43, 0x43, 0x00, 0x0C // "jal 0x80010D0C"
				}
			},
			{
				// Hijack the second call to patch it if required
				.offset = 0x80011E34,
				.size = 4,
				.flags = FLAG_LAST,
				.data = (const uint8_t[]) {
					0x45, 0x43, 0x00, 0x0C // "jal 0x80010D14"
				}
			}
		}
	},
	/*
	 * Legend of Dragoon (E) (Disc 1) (SCES-03043)
	 * Plain antipiracy call in a loop.
	 */
	{
		.crc = 0xD89B3731,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x801C0640,
				.size = 12,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * Legend of Dragoon (F) (Disc 1) (SCES-03044)
	 * Plain antipiracy call in a loop.
	 */
	{
		.crc = 0x7F603194,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x801C0620,
				.size = 12,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * Legend of Dragoon (G) (Disc 1) (SCES-03045)
	 * Plain antipiracy call in a loop.
	 */
	{
		.crc = 0xF72B58F8,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x801C05DC,
				.size = 12,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * Legend of Dragoon (I) (Disc 1) (SCES-03046)
	 * Plain antipiracy call in a loop.
	 */
	{
		.crc = 0x70EFD7A8,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x801C05D4,
				.size = 12,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * Legend of Dragoon (S) (Disc 1) (SCES-03047)
	 * Plain antipiracy call in a loop.
	 */
	{
		.crc = 0xF45338FE,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x801C064C,
				.size = 12,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * Ore No Ryouri (J) (SCPS-10099)
	 * Plain antipiracy. No access to 0xBFC7FF52, but super easy to find by looking for a
	 * function with a blatantly obfuscated flow.
	 */
	{
		.crc = 0xDB2DA23C,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x800330B0,
				.size = 12,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * pop'n music 2 (J) (SLPM-86294)
	 * Antipiracy check at 0x80015318, which gets called from some dynamically loaded code at
	 * 0x800E6F70, which is called from 0x80070828 (the one we'd NOP).
	 */
	{
		.crc = 0x162DC430,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x80070828,
				.size = 4,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * pop'n music 6 (J) (SLPM-87089)
	 * Antipiracy check at 0x80030F64, which gets called from 0x80030EC8.
	 */
	{
		.crc = 0xF4422FAA,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x80030EC8,
				.size = 12,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * Rockman X5 (J) (SLPM-86666)
	 * Copycat of Tomba 2! Boring.
	 */
	{
		.crc = 0xBD535D34,
		.patches = (const struct patch[]) {
			{
				// Nuke call to antipiracy
				.offset = 0x80013230,
				.size = 4,
				.flags = FLAG_NOP | FLAG_LAST
			}
		}
	},
	/*
	 * Seiken Densetsu (J) (SLPS-02170)
	 *
	 * A though one.
	 *
	 * This game, at offset 0x800111DC, calls a function in RAM at 0x8004FE7C, which gets
	 * dynamically decompressed from one of the data files (dunno which).
	 *
	 * This function is a pretty complex one and does a myriad of things, including initializing
	 * the video and the audio, and checking antipiracy using a function at 0x80050EA0. Thus we
	 * cannot stub the entire function call, or it'd crash.
	 *
	 * Instead, we'll use a shim that intercepts the call at 0x800111DC, stubs the calls to
	 * antipiracy, and then continue with the normal program flow.
	 */
	{
		.crc = 0x8F2902AA,
		.patches = (const struct patch[]) {
			{
				// Replace the "jal 0x8004FE7C" with a jal to our patcher
				.offset = 0x800111DC,
				.size = 4,
				.data = (const uint8_t[]) {
					0x5F, 0x42, 0x00, 0x0C, // "jal 0x8001097C"
				}
			},
			{
				// Insert the patcher in place of a debug string
				.offset = 0x8001097C,
				.size = 20,
				.flags = FLAG_LAST,
				.data = (const uint8_t[]) {
					0x02, 0x24, 0x08, 0x3C, // "li $t0, 0x24020000", where the constant is "li $v0, 0"
					0x05, 0x80, 0x09, 0x3C, // "lui $t1, 0x8005"
					0x30, 0x07, 0x28, 0xAD, // "sw $t0, 0x0730($t1)" to insert at 0x80050730 a "li $v0, 0" in place of the original "jal 0x80050EA0"
					0x9F, 0x3F, 0x01, 0x08, // "j 0x8004FE7C" to continue with the normal flow
					0x44, 0x07, 0x28, 0xAD  // "sw $t0, 0x0744($t1)" to insert at 0x80050744 a "li $v0, 0" in place of the original "jal 0x80050EA0"
				}
			}
		}
	},
	/*
	 * Silent Hill Survivor (U) (SLUS-01087)
	 *
	 * This one's main executable just loads KERNEL.BIN and jumps to it, which contains the actual
	 * antipiracy check at 0x80010D2C, which we will just NOP.
	 *
	 * We'd intercept the call to Exec (at 0x80180C94, called from 0x80180A44) after the boot
	 * executable loads it, patch it, and continue with the boot process.
	 */
	{
		.crc = 0x4B2DB3AB,
		.patches = (const struct patch[]) {
			{
				// Replace the "jal 0x80180C94" with a jal to our patcher
				.offset = 0x80180A44,
				.size = 4,
				.data = (const uint8_t[]) {
					0x50, 0x02, 0x06, 0x0C, // "jal 0x80180940"
				}
			},
			{
				// Insert the patcher in place of a debug string
				.offset = 0x80180940,
				.size = 12,
				.flags = FLAG_LAST,
				.data = (const uint8_t[]) {
					0x01, 0x80, 0x08, 0x3C, // "lui $t0, 0x8001"
					0x25, 0x03, 0x06, 0x08, // "j 0x80180C94" to continue with the normal flow
					0x2C, 0x0D, 0x00, 0xAD, // "sw $zero, 0x0D2C($t0)" to nop the antipiracy call
				}
			}
		}
	},
	/*
	 * Tetris with Card Captor Sakura - Eternal Heart (J) (SLPS-02886)
	 * Plain call, but this one checks the function return value.
	 */
	{
		.crc = 0x8E11C761,
		.patches = (const struct patch[]) {
			{
				 // Replace the call with a "ori v0,0,$1"
				.offset = 0x8001FFC4,
				.size = 4,
				.flags = FLAG_LAST,
				.data = (const uint8_t[]) { 0x01, 0x00, 0x02, 0x34 }
			}
		}
	},
	/*
	 * Tokimeki Memorial 2 (J) (Disc 1) (SLPM-86355)
	 *
	 * Call is loaded obfuscated from CDPACK00.BIN offset 0x800, and gets loaded to 0x8001000
	 * via CD-ROM DMA (note that write breakpoints don't work in no$psx for DMAs).
	 *
	 * After loading it, the CPU deobfuscates it by adding 0x9C (modulo 256) to every byte. Next,
	 * the antipiracy main function (at 0x8001146C) gets called from 0x80017A24.
	 *
	 * Nuking the call isn't enough as this function sets some global variables:
	 *  *((uint32_t *) 0x8006C69C) = 0x21D; <<< in .data section
	 *  *((uint16_t *) 0x80076BEC) |= 0x8000; <<< in .bss section
	 *  *((uint8_t *)  0x80076BEF) = 1; <<< in .bss section
	 *
	 * Without setting the first the game crashes with an invalid opcode exception, but the later
	 * don't seem to make any difference. Given they are in .bss and they'd get wiped out from
	 * RAM if we set them ourselves, we'll set the first only one and pray the last two aren't
	 * part of some convoluted, Spyro-3-like booby trap.
	 */
	{
		.crc = 0x9F44049C,
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
	 * Tokimeki Memorial 2 [Konami The Best] (J) (Disc 1) (SLPM-86723)
	 */
	{
		.crc = 0x16CF7197,
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
				.offset = 0x8006C708,
				.size = 2,
				.flags = FLAG_LAST,
				.data = (const uint8_t[]) { 0x1D, 0x02 }
			}
		}
	},
	/*
	 * Tomba! 2 - The Evil Swine Return (U) (SCUS-94454)
	 * Easy to find by looking for reads to address 0xBFC7FF52 (the one the antipiracy uses to
	 * check the BIOS region).
	 */
	{
		.crc = 0xD88B9630,
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
	 * YuGiOh Forbidden Memories (U) (SLUS-01411)
	 * This game has the check function in the WA_MRG.MRG file.
	 * It is loaded via CDROM DMA (so don't try to intercept the read via memory write breakpoints)
	 */
	{
		.crc = 0x96382505,
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
	 * YuGiOh Forbidden Memories (S) (SLES-03951)
	 */
	{
		.crc = 0x9DA374C3,
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

	debug_write("Patching");
	const struct patch * cur_patch = game->patches;
	while (1) {
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
