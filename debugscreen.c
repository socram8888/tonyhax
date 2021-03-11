
#include "debugscreen.h"
#include "gpu.h"
#include "bios.h"
#include "str.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CHAR_HEIGHT 15
#define CHAR_WIDTH 8
#define FONT_X 512
#define CLUT_X 512
#define CLUT_Y 30

#define LOG_MARGIN 10
#define LOG_START_Y 35
#define LOG_LINE_HEIGHT 16

static uint_fast8_t log_lines;

void loadfont() {
	// Font is 1bpp. We have to convert each character to 4bpp.
	const uint8_t * rom_charset = (const uint8_t *) 0xBFC7F8DE;
	uint8_t charbuf[60];

	// Iterate through the 16x6 character table
	for (uint_fast8_t tabley = 0; tabley < 6; tabley++) {
		for (uint_fast8_t tablex = 0; tablex < 16; tablex++) {
			uint8_t * bufferpos = charbuf;

			// Iterate through each line of the 8x15 character
			for (uint_fast8_t chary = 0; chary < 15; chary++) {
				uint_fast8_t char1bpp = *rom_charset;
				rom_charset++;

				// Iterate through each column of the character
				for (uint_fast8_t bpos = 0; bpos < 8; bpos += 2) {
					uint_fast8_t char4bpp = 0;

					if (char1bpp & 0x80) {
						char4bpp |= 0x01;
					}
					if (char1bpp & 0x40) {
						char4bpp |= 0x10;
					}

					*bufferpos = char4bpp;
					bufferpos++;
					char1bpp = char1bpp << 2;
				}
			}

			// At 4bpp, each character uses 8 * 4 / 16 = 2 shorts, so the texture width is set to 2.
			GPU_dw(FONT_X + tablex * 2, tabley * CHAR_HEIGHT, 2, CHAR_HEIGHT, (uint16_t *) charbuf);
		}
	}
}

void loadclut() {
	uint16_t colors[16];
	// Load CLUT

	for (int i = 0; i < 16; i++) {
		// Black
		colors[i] = 0x0000;
	}

	// Make 1 white
	colors[1] = 0x7FFF;

	// Load the palette to Vram
	GPU_dw(CLUT_X, CLUT_Y, 16, 1, colors);
}

void debug_init() {
	bool pal = gpu_is_pal();

	gpu_reset();

	// Configure mode, keeping PAL flag
	uint32_t mode = GPU_DISPLAY_H320 | GPU_DISPLAY_V240 | GPU_DISPLAY_15BPP;
	if (pal) {
		mode |= GPU_DISPLAY_PAL;

		log_lines = 13;
	} else {
		mode |= GPU_DISPLAY_NTSC;

		log_lines = 12;
	}
	gpu_display_mode(mode);

	// Center image on screen
	// Values from THPS2 NTSC and PAL during FMVs
	if (pal) {
		gpu_set_hrange(624, 3260);
		gpu_set_vrange(37, 292);
	} else {
		gpu_set_hrange(600, 3160);
		gpu_set_vrange(16, 256);
	}

	// Clear entire VRAM
	gpu_fill_rectangle(0, 0, 1023, 511, 0x000000);

	// Enable display
	gpu_display(true);

	// Load font
	loadfont();
	loadclut();

	// Flush old textures
	gpu_flush_cache();

	// Configure Texpage
	// - Texture page to X=512 Y=0
	// - Colors to 4bpp
	// - Allow drawing to display area (fuck Vsync)
	GPU_cw(0xE1000408);

	// Configure texture window
	GPU_cw(0xE2000000);

	// Set drawing area
	gpu_set_drawing_area(0, 0, 256, 256);

	// Draw border
	debug_text_at(20, 10, "tonyhax v1.0");
	gpu_fill_rectangle(0, 30, SCREEN_WIDTH, 2, 0xFFFFFF);
}

void debug_text_at(uint_fast16_t x_pos, uint_fast16_t y_pos, const char * text) {
	// Initialize constants of the rect
	struct gpu_tex_rect rect;
	rect.draw_y = y_pos;
	rect.width = CHAR_WIDTH;
	rect.height = CHAR_HEIGHT;
	rect.clut_x = CLUT_X;
	rect.clut_y = CLUT_Y;
	rect.semi_transp = 0;
	rect.raw_tex = 1;

	while (*text != 0) {
		int tex_idx = *text - '!';
		if (tex_idx >= 0 && tex_idx < 96) {
			// Font has a yen symbol where the \ should be
			if (tex_idx == '\\' - '!') {
				tex_idx = 95;
			}

			// Draw text
			rect.draw_x = x_pos;
			rect.tex_x = (tex_idx % 16) * CHAR_WIDTH;
			rect.tex_y = (tex_idx / 16) * CHAR_HEIGHT;

			gpu_draw_tex_rect(&rect);
		}

		text++;
		x_pos += CHAR_WIDTH;
	}
}

void debug_write(const char * str, ...) {
	va_list args;
	va_start(args, str);

	// For a render width of 320 this should be just 40 but let's be generous
	char formatted[64];

	mini_vsprintf(formatted, str, args);

	// Scroll text up
	for (int line = 1; line < log_lines; line++) {
		gpu_copy_rectangle(
				/* source */
				LOG_MARGIN,
				LOG_START_Y + LOG_LINE_HEIGHT * line,
				
				/* destination */
				LOG_MARGIN,
				LOG_START_Y + LOG_LINE_HEIGHT * (line - 1),
				
				/* size */
				SCREEN_WIDTH - LOG_MARGIN,
				LOG_LINE_HEIGHT
		);
	}

	// Clear last line
	gpu_fill_rectangle(0, LOG_START_Y + (log_lines - 1) * LOG_LINE_HEIGHT, SCREEN_WIDTH, CHAR_HEIGHT, 0x000000);

	// Draw text on last line
	debug_text_at(LOG_MARGIN, LOG_START_Y + (log_lines - 1) * LOG_LINE_HEIGHT, formatted);
}
