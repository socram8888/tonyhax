
#include "debugscreen.h"
#include "gpu.h"
#include "bios.h"
#include "str.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define CHAR_HEIGHT 15
#define CHAR_WIDTH 8
#define FONT_X 512
#define CLUT_X 512
#define CLUT_Y 30

#define TEXT_START_Y (SCREEN_HEIGHT / 2 - CHAR_WIDTH)

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
	uint32_t mode = GPU_DISPLAY_H256 | GPU_DISPLAY_V240 | GPU_DISPLAY_15BPP;
	if (pal) {
		mode |= GPU_DISPLAY_PAL;
	} else {
		mode |= GPU_DISPLAY_NTSC;
	}
	gpu_display_mode(mode);

	// Clear entire VRAM
	gpu_fill_rectangle(0, 0, 1023, 511, 0x200000);

	// Enable display
	gpu_display(true);

	// Load font
	loadfont();
	loadclut();

	gpu_flush_cache();
}

void debug_write(const char * str, ...) {

	va_list args;
	va_start(args, str);

	// For a render width of 256 this should be just 32 but let's be generous
	char formatted[64];

	int32_t len = mini_vsprintf(formatted, str, args);

	// Clear text section
	gpu_fill_rectangle(0, TEXT_START_Y, SCREEN_WIDTH, CHAR_HEIGHT, 0x000000);

	// Configure Texpage
	// - Texture page to X=512 Y=0
	// - Colors to 4bpp
	// - Allow drawing to display area (fuck Vsync)
	GPU_cw(0xE1000408);

	// Configure texture window
	GPU_cw(0xE2000000);

	// Set drawing area
	gpu_set_drawing_area(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Configure drawing offset (none)
	GPU_cw(0xE5000000);

	// Disable drawing mask
	GPU_cw(0xE6000000);

	printf("Printing \"%s\"\n", formatted);
	uint32_t x_pos = SCREEN_WIDTH / 2 - len * CHAR_WIDTH / 2;

	// Initialize constants of the rect
	struct gpu_tex_rect rect;
	rect.draw_y = TEXT_START_Y;
	rect.width = CHAR_WIDTH;
	rect.height = CHAR_HEIGHT;
	rect.clut_x = CLUT_X;
	rect.clut_y = CLUT_Y;
	rect.semi_transp = 0;
	rect.raw_tex = 1;

	for (int i = 0; i < len; i++) {
		int tex_idx = formatted[i] - '!';
		if (tex_idx >= 0 && tex_idx < 96) {
			// Draw text
			rect.draw_x = x_pos;
			rect.tex_x = (tex_idx % 16) * CHAR_WIDTH;
			rect.tex_y = (tex_idx / 16) * CHAR_HEIGHT;

			gpu_draw_tex_rect(&rect);
		}

		x_pos += CHAR_WIDTH;
	}
}
