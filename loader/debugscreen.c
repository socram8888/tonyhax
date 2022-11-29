
#include "str.h"
#include "debugscreen.h"
#include "gpu.h"
#include "bios.h"
#include "str.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CHAR_HEIGHT 15
#define CHAR_DRAW_WIDTH 10
#define CHAR_VRAM_WIDTH 8
#define FONT_X 640
#define CLUT_X 640
#define CLUT_Y (6 * CHAR_HEIGHT)

// Orca loaded right next to the font
#define ORCA_WIDTH 40
#define ORCA_HEIGHT 20

// Divided by 4 because each pixel is 4bpp, or 1/4 of a 16-bit short
#define ORCA_TEXCOORD_X (CHAR_VRAM_WIDTH * 16)

#define TH_MARGIN 40
#define LOG_LINES 22
#define LOG_MARGIN 32
#define LOG_START_Y 80
#define LOG_LINE_HEIGHT 16

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

// Grayscale
static const uint16_t PALETTE[16] = { 0x0000, 0x0842, 0x1084, 0x18C6, 0x2108, 0x294A, 0x318C, 0x39CE, 0x4631, 0x4E73, 0x56B5, 0x5EF7, 0x6739, 0x6F7B, 0x77BD, 0x7FFF };

#include "orca.inc"

void decompressfont() {
	// Font is 1bpp. We have to convert each character to 4bpp.
	const uint8_t * rom_charset = (const uint8_t *) 0xBFC7F8DE;
	uint8_t charbuf[CHAR_HEIGHT * CHAR_VRAM_WIDTH / 2];

	// Iterate through the 16x6 character table
	for (uint_fast8_t tabley = 0; tabley < 6; tabley++) {
		for (uint_fast8_t tablex = 0; tablex < 16; tablex++) {
			uint8_t * bufferpos = charbuf;

			// Iterate through each line of the 8x15 character
			for (uint_fast8_t chary = 0; chary < 15; chary++) {
				uint_fast8_t char1bpp = *rom_charset;
				uint_fast8_t bpos = 0;
				rom_charset++;

				// Iterate through each column of the character
				while (bpos < 8) {
					uint_fast8_t char4bpp = 0;

					if (char1bpp & 0x80) {
						char4bpp |= 0x0F;
					}
					bpos++;

					if (char1bpp & 0x40) {
						char4bpp |= 0xF0;
					}
					bpos++;

					*bufferpos = char4bpp;
					bufferpos++;
					char1bpp = char1bpp << 2;
				}
			}

			// GPU_dw takes units in 16bpp units, so we have to scale by 1/4
			GPU_dw(FONT_X + tablex * CHAR_VRAM_WIDTH * 4 / 16, tabley * CHAR_HEIGHT, CHAR_VRAM_WIDTH * 4 / 16, CHAR_HEIGHT, (uint16_t *) charbuf);
		}
	}
}

char last_printed_line[64];
uint32_t last_printed_count;

void debug_init() {
	bool pal = bios_is_european();
	debug_switch_standard(pal);

	// Clear display
	struct gpu_solid_rect background = {
		.pos = {
			.x = 0,
			.y = 0,
		},
		.size = {
			.width = SCREEN_WIDTH,
			.height = SCREEN_HEIGHT,
		},
		.color = 0x000000,
		.semi_transp = 0,
	};
	gpu_draw_solid_rect(&background);

	// Load font
	decompressfont();

	// Load orca image
	// Again, /4 because each pixels is 1/4 of a 16-bit short
	GPU_dw(FONT_X + ORCA_TEXCOORD_X / 4, 0, ORCA_WIDTH / 4, ORCA_HEIGHT, (const uint16_t *) ORCA_IMAGE);

	// Load the palette to Vram
	GPU_dw(CLUT_X, CLUT_Y, 16, 1, PALETTE);

	// Flush old textures
	gpu_flush_cache();

	// Draw border
	debug_text_at(TH_MARGIN, 40, "tonyhax " STRINGIFY(TONYHAX_VERSION));
	struct gpu_solid_rect band = {
		.pos = {
			.x = 0,
			.y = 65,
		},
		.size = {
			.width = SCREEN_WIDTH,
			.height = 2,
		},
		.color = 0xFFFFFF,
		.semi_transp = 0,
	};
	gpu_draw_solid_rect(&band);

	// "orca.pet" website
	debug_text_at(SCREEN_WIDTH - 8 * CHAR_DRAW_WIDTH - TH_MARGIN, 40, "orca.pet");

	// Draw orca
	gpu_tex_rect_t orca_rect = {
		.texcoord = {
			.x = ORCA_TEXCOORD_X,
			.y = 0,
		},
		.pos = {
			.x = SCREEN_WIDTH - 8 * CHAR_DRAW_WIDTH - TH_MARGIN - 10 - ORCA_WIDTH,
			.y = 40,
		},
		.size = {
			.width = ORCA_WIDTH,
			.height = ORCA_HEIGHT,
		},
		.clut = {
			.x = CLUT_X,
			.y = CLUT_Y,
		},
		.semi_transp = 0,
		.raw_tex = 1,
	};
	gpu_draw_tex_rect(&orca_rect);
}

void debug_text_at(uint_fast16_t x_pos, uint_fast16_t y_pos, const char * text) {
	// Initialize constants of the rect
	gpu_tex_rect_t rect = {
		.pos = {
			.y = y_pos,
		},
		.size = {
			.width = CHAR_VRAM_WIDTH,
			.height = CHAR_HEIGHT,
		},
		.clut = {
			.x = CLUT_X,
			.y = CLUT_Y,
		},
		.semi_transp = 0,
		.raw_tex = 1,
	};

	while (*text != 0) {
		int tex_idx = *text - '!';
		if (tex_idx >= 0 && tex_idx < 96) {
			// Font has a yen symbol where the \ should be
			if (tex_idx == '\\' - '!') {
				tex_idx = 95;
			}

			// Draw text
			rect.pos.x = x_pos;
			rect.texcoord.x = (tex_idx % 16) * CHAR_VRAM_WIDTH;
			rect.texcoord.y = (tex_idx / 16) * CHAR_HEIGHT;
			gpu_draw_tex_rect(&rect);

			// Again to overstrike and improve visibility
			rect.pos.x++;
			gpu_draw_tex_rect(&rect);
		}

		x_pos += CHAR_DRAW_WIDTH;
		text++;
	}
}

void debug_write(const char * str, ...) {
	va_list args;
	va_start(args, str);

	// For a render width of 640 at width 10 max line should be 64 chars long
	char formatted[64];
	char formatted_repeated[64];
	char * to_print;
	mini_vsprintf(formatted, str, args);
	va_end(args);

	// Flush old textures
	gpu_flush_cache();

	if (strcmp(last_printed_line, formatted) != 0) {
		// Line's text is different, so scroll up
		gpu_size_t line_size = {
			.width = SCREEN_WIDTH - LOG_MARGIN,
			.height = LOG_LINE_HEIGHT,
		};
		for (int line = 1; line < LOG_LINES; line++) {
			gpu_point_t source_line = {
				.x = LOG_MARGIN,
				.y = LOG_START_Y + LOG_LINE_HEIGHT * line
			};
			gpu_point_t dest_line = {
				.x = LOG_MARGIN,
				.y = LOG_START_Y + LOG_LINE_HEIGHT * (line - 1)
			};
			gpu_copy_rectangle(&source_line, &dest_line, &line_size);
		}

		// Copy to last printed buffer and reset counter
		strcpy(last_printed_line, formatted);
		last_printed_count = 1;

		to_print = formatted;
	} else {
		last_printed_count++;

		// Same line, so print with a repeat counter
		mini_sprintf(formatted_repeated, "%s (x%d)", last_printed_line, last_printed_count);

		to_print = formatted_repeated;
	}

	uint32_t lastLinePos = LOG_START_Y + (LOG_LINES - 1) * LOG_LINE_HEIGHT;

	// Clear last line
	gpu_solid_rect_t black_box = {
		.pos = {
			.x = LOG_MARGIN,
			.y = lastLinePos,
		},
		.size = {
			.width = SCREEN_WIDTH - LOG_MARGIN,
			.height = CHAR_HEIGHT,
		},
		.color = 0x000000,
		.semi_transp = 0,
	};
	gpu_draw_solid_rect(&black_box);

	// Draw text on last line
	debug_text_at(LOG_MARGIN, lastLinePos, to_print);
}

void debug_switch_standard(bool pal) {
	// Restore to sane defaults
	gpu_reset();

	// Configure mode, keeping PAL flag
	uint32_t mode = GPU_DISPLAY_H640 | GPU_DISPLAY_V480 | GPU_DISPLAY_15BPP;
	if (pal) {
		mode |= GPU_DISPLAY_PAL;
	} else {
		mode |= GPU_DISPLAY_NTSC;
	}
	gpu_display_mode(mode);

	// Center image on screen
	// Values from BIOSes
	if (pal) {
		gpu_set_hrange(638, 3198);
		gpu_set_vrange(43, 282);
	} else {
		gpu_set_hrange(608, 3168);
		gpu_set_vrange(16, 255);
	}

	// Set drawing area for entire display port
	gpu_point_t area_start = { 0, 0 };
	gpu_size_t area_size = { 640, 480 };
	gpu_set_drawing_area(&area_start, &area_size);

	// Enable display
	gpu_display_enable();

	// Configure Texpage
	// - Texture page to X=640 Y=0
	// - Colors to 4bpp
	// - Allow drawing to display area (fuck Vsync)
	GPU_cw(0xE100040A);

	// Configure texture window
	GPU_cw(0xE2000000);
}
