
#include "debugscreen.h"
#include "gpu.h"
#include "bios.h"
#include "str.h"

#define SCALE(x) ((x) + (x) / 2)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CHAR_HEIGHT 15
#define CHAR_WIDTH 8
#define FONT_X 640
#define CLUT_X 640
#define CLUT_Y (6 * CHAR_HEIGHT)

// Orca loaded right next to the font
#define ORCA_WIDTH 40
#define ORCA_HEIGHT 20

// Divided by 4 because each pixel is 4bpp, or 1/4 of a 16-bit short
#define ORCA_VRAM_X (FONT_X + CHAR_WIDTH * 16 / 4)

#define TH_MARGIN 40
#define LOG_LINES 14
#define LOG_MARGIN 32
#define LOG_START_Y 80
#define LOG_LINE_HEIGHT SCALE(CHAR_HEIGHT + 1)

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
				for (uint_fast8_t bpos = 0; bpos < 8; bpos += 2) {
					uint_fast8_t char4bpp = 0;

					if (char1bpp & 0x80) {
						char4bpp |= 0x0F;
					}
					if (char1bpp & 0x40) {
						char4bpp |= 0xF0;
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

void draw_scaled_rect(const struct gpu_point * font_offset, const struct gpu_point * size, const struct gpu_point * dst) {
	gpu_tex_poly_t poly;

	poly.vertex_count = 4;
	poly.color_mode = GPU_COLORMODE_4BPP;
	poly.semi_transp = GPU_SEMITRANSP_OFF;
	poly.raw_tex = 1;
	poly.clut.x = CLUT_X;
	poly.clut.y = CLUT_Y;
	poly.tex_base.x = FONT_X;
	poly.tex_base.y = 0;

	poly.vertex[0].x = dst->x;
	poly.vertex[0].y = dst->y;

	poly.vertex[1].x = dst->x + SCALE(size->x);
	poly.vertex[1].y = dst->y;

	poly.vertex[2].x = dst->x;
	poly.vertex[2].y = dst->y + SCALE(size->y);

	poly.vertex[3].x = dst->x + SCALE(size->x);
	poly.vertex[3].y = dst->y + SCALE(size->y);

	poly.tex_coord[0].x = font_offset->x;
	poly.tex_coord[0].y = font_offset->y;

	poly.tex_coord[1].x = font_offset->x + size->x;
	poly.tex_coord[1].y = font_offset->y;

	poly.tex_coord[2].x = font_offset->x;
	poly.tex_coord[2].y = font_offset->y + size->y;

	poly.tex_coord[3].x = font_offset->x + size->x;
	poly.tex_coord[3].y = font_offset->y + size->y;

	gpu_draw_tex_poly(&poly);
}

void debug_init() {
	bool pal = bios_is_european();
	gpu_init_bios(pal);

	// Configure Texpage
	// - Texture page to X=640 Y=0
	// - Colors to 4bpp
	// - Allow drawing to display area (fuck Vsync)
	GPU_cw(0xE100040A);

	// Configure texture window
	GPU_cw(0xE2000000);

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
	GPU_dw(ORCA_VRAM_X, 0, ORCA_WIDTH / 4, ORCA_HEIGHT, (const uint16_t *) ORCA_IMAGE);

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
	debug_text_at(SCREEN_WIDTH - 8 * SCALE(CHAR_WIDTH) - TH_MARGIN, 40, "orca.pet");

	// Draw orca
	struct gpu_point orca_tex, orca_size, orca_dst;
	orca_tex.x = 16 * CHAR_WIDTH;
	orca_tex.y = 0;
	orca_size.x = ORCA_WIDTH;
	orca_size.y = ORCA_HEIGHT;
	orca_dst.x = SCREEN_WIDTH - 8 * SCALE(CHAR_WIDTH) - 20 - SCALE(ORCA_WIDTH) - 10;
	orca_dst.y = 5;
	draw_scaled_rect(&orca_tex, &orca_size, &orca_dst);
}

void debug_text_at(uint_fast16_t x_pos, uint_fast16_t y_pos, const char * text) {
	struct gpu_point font_offset, size, dst;
	size.x = CHAR_WIDTH;
	size.y = CHAR_HEIGHT;
	dst.x = x_pos;
	dst.y = y_pos;

	while (*text != 0) {
		int tex_idx = *text - '!';
		if (tex_idx >= 0 && tex_idx < 96) {
			// Font has a yen symbol where the \ should be
			if (tex_idx == '\\' - '!') {
				tex_idx = 95;
			}

			// Draw text
			font_offset.x = (tex_idx % 16) * CHAR_WIDTH;
			font_offset.y = (tex_idx / 16) * CHAR_HEIGHT;

			draw_scaled_rect(&font_offset, &size, &dst);
		}

		text++;
		dst.x += SCALE(CHAR_WIDTH);
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

	uint32_t lastLinePos = LOG_START_Y + (LOG_LINES - 1) * LOG_LINE_HEIGHT;

	// Clear last line
	gpu_solid_rect_t black_box = {
		.pos = {
			.x = LOG_MARGIN,
			.y = lastLinePos,
		},
		.size = {
			.width = SCREEN_WIDTH - LOG_MARGIN,
			.height = LOG_LINE_HEIGHT,
		},
		.color = 0x000000,
		.semi_transp = 0,
	};
	gpu_draw_solid_rect(&black_box);

	// Draw text on last line
	debug_text_at(LOG_MARGIN, lastLinePos, formatted);
}
