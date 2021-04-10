
#include "debugscreen.h"
#include "gpu.h"
#include "bios.h"
#include "str.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CHAR_HEIGHT 15
#define CHAR_ROM_WIDTH 8
#define CHAR_DRAW_WIDTH (CHAR_ROM_WIDTH + 2)
#define FONT_X 640
#define CLUT_X 640
#define CLUT_Y (6 * CHAR_HEIGHT)

// Orca loaded right next to the font
#define ORCA_WIDTH 40
#define ORCA_HEIGHT 20

// Divided by 4 because each pixel is 4bpp, or 1/4 of a 16-bit short
#define ORCA_VRAM_X (FONT_X + CHAR_ROM_WIDTH * 16 / 4)

#define TH_MARGIN 40
#define LOG_LINES 22
#define LOG_MARGIN 30
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
				bool last_black = false;
				for (uint_fast8_t bpos = 0; bpos < 8; bpos += 2) {
					uint_fast8_t char4bpp = 0;

					if (last_black) {
						char4bpp |= 0x0F;
						last_black = false;
					}
					if (char1bpp & 0x80) {
						char4bpp |= 0xFF;
					}
					if (char1bpp & 0x40) {
						char4bpp |= 0xF0;
						last_black = true;
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

void debug_init() {
	bool pal = gpu_is_pal();
	gpu_init_bios(pal);

	// Clear entire VRAM
	gpu_fill_rectangle(0, 0, 1023, 511, 0x000000);

	// Load font
	decompressfont();

	// Load orca image
	// Again, /4 because each pixels is 1/4 of a 16-bit short
	GPU_dw(ORCA_VRAM_X, 0, ORCA_WIDTH / 4, ORCA_HEIGHT, (const uint16_t *) ORCA_IMAGE);

	// Load the palette to Vram
	GPU_dw(CLUT_X, CLUT_Y, 16, 1, PALETTE);

	// Flush old textures
	gpu_flush_cache();

	// Configure Texpage
	// - Texture page to X=640 Y=0
	// - Colors to 4bpp
	// - Allow drawing to display area (fuck Vsync)
	GPU_cw(0xE100040A);

	// Configure texture window
	GPU_cw(0xE2000000);

	// Draw border
	debug_text_at(TH_MARGIN, 40, "tonyhax " STRINGIFY(TONYHAX_VERSION));
	gpu_fill_rectangle(0, 65, SCREEN_WIDTH, 2, 0xFFFFFF);

	// "orca.pet" website
	debug_text_at(SCREEN_WIDTH - 8 * CHAR_DRAW_WIDTH - TH_MARGIN, 40, "orca.pet");

	// Draw orca
	struct gpu_tex_rect orca_rect;
	orca_rect.texcoord.x = 16 * CHAR_ROM_WIDTH;
	orca_rect.texcoord.y = 0;
	orca_rect.width = ORCA_WIDTH;
	orca_rect.height = ORCA_HEIGHT;
	orca_rect.pos.x = SCREEN_WIDTH - 8 * CHAR_DRAW_WIDTH - TH_MARGIN - 10 - ORCA_WIDTH;
	orca_rect.pos.y = 40;
	orca_rect.clut.x = CLUT_X;
	orca_rect.clut.y = CLUT_Y;
	orca_rect.semi_transp = 0;
	orca_rect.raw_tex = 1;
	gpu_draw_tex_rect(&orca_rect);
}

void debug_text_at(uint_fast16_t x_pos, uint_fast16_t y_pos, const char * text) {
	// Initialize constants of the rect
	struct gpu_tex_rect rect;
	rect.pos.y = y_pos;
	rect.width = CHAR_ROM_WIDTH;
	rect.height = CHAR_HEIGHT;
	rect.clut.x = CLUT_X;
	rect.clut.y = CLUT_Y;
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
			rect.pos.x = x_pos;
			rect.texcoord.x = (tex_idx % 16) * CHAR_ROM_WIDTH;
			rect.texcoord.y = (tex_idx / 16) * CHAR_HEIGHT;

			gpu_draw_tex_rect(&rect);
		}

		text++;
		x_pos += CHAR_DRAW_WIDTH;
	}
}

void debug_write(const char * str, ...) {
	va_list args;
	va_start(args, str);

	// For a render width of 640 this should be just 40 but let's be generous
	char formatted[128];

	mini_vsprintf(formatted, str, args);

	// Flush old textures
	gpu_flush_cache();

	// Scroll text up
	for (int line = 1; line < LOG_LINES; line++) {
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
	gpu_fill_rectangle(0, LOG_START_Y + (LOG_LINES - 1) * LOG_LINE_HEIGHT, SCREEN_WIDTH, CHAR_HEIGHT, 0x000000);

	// Draw text on last line
	debug_text_at(LOG_MARGIN, LOG_START_Y + (LOG_LINES - 1) * LOG_LINE_HEIGHT, formatted);
}
