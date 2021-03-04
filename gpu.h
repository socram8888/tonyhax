
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define GPU_DISPLAY_H256 0
#define GPU_DISPLAY_H320 1
#define GPU_DISPLAY_H512 2
#define GPU_DISPLAY_H640 3

#define GPU_DISPLAY_V240 (0 << 2)
#define GPU_DISPLAY_V480 (1 << 2)

#define GPU_DISPLAY_NTSC (0 << 3)
#define GPU_DISPLAY_PAL  (1 << 3)

#define GPU_DISPLAY_15BPP (0 << 4)
#define GPU_DISPLAY_24BPP (1 << 4)

#define GPU_DISPLAY_INTERLACED (1 << 5)

struct gpu_tex_rect {
	uint32_t color;
	uint16_t draw_x;
	uint16_t draw_y;
	uint16_t width;
	uint16_t height;
	uint16_t clut_x;
	uint16_t clut_y;
	uint8_t tex_x;
	uint8_t tex_y;
	uint8_t semi_transp;
	uint8_t raw_tex;
};

void gpu_reset(void);

bool gpu_is_pal(void);

void gpu_display(bool enable);

void gpu_display_mode(uint32_t mode);

void gpu_fill_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t rgb);

void gpu_draw_tex_rect(const struct gpu_tex_rect * rect);

void gpu_set_drawing_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void gpu_flush_cache(void);
