
#include "gpu.h"
#include "bios.h"

#define GPU_GP0_FLUSH 0x01
#define GPU_GP0_FILL_VRAM 0x02
#define GPU_GP0_VRAM_TO_VRAM 0x80

#define GPU_GP1_RESET 0x00
#define GPU_GP1_DISPLAY_ENABLE 0x03
#define GPU_GP1_H_RANGE 0x06
#define GPU_GP1_V_RANGE 0x07
#define GPU_GP1_DISPLAY_MODE 0x08

volatile uint32_t * const GPU_STAT = (volatile uint32_t *) 0x1F801814;

bool gpu_is_pal(void) {
	return (*GPU_STAT & (1 << 20)) != 0;
}

void gpu_reset(void) {
	SendGP1Command(GPU_GP1_RESET << 24);
}

void gpu_display_mode(uint32_t mode) {
	SendGP1Command(GPU_GP1_DISPLAY_MODE << 24 | mode);
}

void gpu_display_enable(void) {
	SendGP1Command(GPU_GP1_DISPLAY_ENABLE << 24 | 0);
}

void gpu_display_disable(void) {
	SendGP1Command(GPU_GP1_DISPLAY_ENABLE << 24 | 1);
}

void gpu_fill_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t rgb) {
	uint8_t r = rgb >> 16;
	uint8_t g = rgb >> 8;
	uint8_t b = rgb >> 0;

	uint32_t buf[3];
	buf[0] = GPU_GP0_FILL_VRAM << 24 | (uint32_t) b << 16 | (uint32_t) g << 8 | (uint32_t) r;
	buf[1] = (uint32_t) y << 16 | x;
	buf[2] = (uint32_t) height << 16 | width;
	GPU_cwp(buf, 3);
}

void gpu_copy_rectangle(uint16_t src_x, uint16_t src_y, uint16_t dest_x, uint16_t dest_y, uint16_t width, uint16_t height) {
	uint32_t buf[4];
	buf[0] = GPU_GP0_VRAM_TO_VRAM << 24;
	buf[1] = (uint32_t) src_y << 16 | src_x;
	buf[2] = (uint32_t) dest_y << 16 | dest_x;
	buf[3] = (uint32_t) height << 16 | width;
	GPU_cwp(buf, 4);
}

void gpu_draw_tex_rect(const struct gpu_tex_rect * rect) {
	uint32_t buf[4];

	if (rect->raw_tex) {
		buf[0] = 0x65000000;
	} else {
		uint8_t r = rect->color >> 16;
		uint8_t g = rect->color >> 8;
		uint8_t b = rect->color >> 0;

		buf[0] = 0x64000000 | (uint32_t) b << 16 | (uint32_t) g << 8 | (uint32_t) r;
	}

	if (rect->semi_transp) {
		buf[0] |= 0x02000000;
	}

	buf[1] = (uint32_t) rect->pos.y << 16 | rect->pos.x;
	buf[2] = (uint32_t) rect->clut.y << 22 | (uint32_t) (rect->clut.x / 16) << 16 | (uint32_t) rect->texcoord.y << 8 | rect->texcoord.x;
	buf[3] = (uint32_t) rect->height << 16 | rect->width;

	GPU_cwp(buf, 4);
}

void gpu_draw_tex_poly(const struct gpu_tex_poly * poly) {
	uint32_t buf[9];

	if (poly->raw_tex) {
		buf[0] = 0x25000000;
	} else {
		uint8_t r = poly->color >> 16;
		uint8_t g = poly->color >> 8;
		uint8_t b = poly->color >> 0;

		buf[0] = 0x24000000 | (uint32_t) b << 16 | (uint32_t) g << 8 | (uint32_t) r;
	}

	if (poly->semi_transp) {
		buf[0] |= 0x02000000;
	}

	if (poly->vertex_count == 4) {
		buf[0] |= 0x08000000;
	}

	buf[1] = (uint32_t) poly->vertex[0].y << 16 | poly->vertex[0].x;
	buf[2] = (uint32_t) poly->clut.y << 22 | (uint32_t) (poly->clut.x / 16) << 16 | (uint32_t) poly->texcoord[0].y << 8 | poly->texcoord[0].x;
	buf[3] = (uint32_t) poly->vertex[1].y << 16 | poly->vertex[1].x;
	buf[4] = (uint32_t) (poly->texpage.y / 256) << 20 | (uint32_t) (poly->texpage.x / 64) << 16 | (uint32_t) poly->texcoord[1].y << 8 | poly->texcoord[0].x;
	buf[5] = (uint32_t) poly->vertex[2].y << 16 | poly->vertex[2].x;
	buf[6] = (uint32_t) poly->texcoord[2].y << 8 | poly->texcoord[2].x;

	if (poly->vertex_count == 4) {
		buf[7] = (uint32_t) poly->vertex[3].y << 16 | poly->vertex[3].x;
		buf[8] = (uint32_t) poly->texcoord[3].y << 8 | poly->texcoord[3].x;

		GPU_cwp(buf, 9);
	} else {
		GPU_cwp(buf, 7);
	}
}

void gpu_set_drawing_area(uint_fast16_t x, uint_fast16_t y, uint_fast16_t width, uint_fast16_t height) {
	GPU_cw(0xE3000000 | y << 10 | x);
	GPU_cw(0xE4000000 | (y + height - 1) << 10 | (x + width - 1));
}

void gpu_flush_cache(void) {
	GPU_cw(GPU_GP0_FLUSH << 24);
}

void gpu_set_hrange(uint_fast16_t x1, uint_fast16_t x2) {
	SendGP1Command(GPU_GP1_H_RANGE << 24 | x2 << 12 | x1);
}

void gpu_set_vrange(uint_fast16_t y1, uint_fast16_t y2) {
	SendGP1Command(GPU_GP1_V_RANGE << 24 | y2 << 10 | y1);
}

void gpu_init_bios(bool pal) {
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

	// Set drawing area (same as BIOS)
	gpu_set_drawing_area(0, 2, 640, 478);

	// Enable display
	gpu_display_enable();
}
