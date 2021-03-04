
#pragma once
#include <stdbool.h>
#include <stdint.h>

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

void gpu_display(bool enable);

void gpu_fill_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t rgb);

void gpu_draw_tex_rect(const struct gpu_tex_rect * rect);

void gpu_set_drawing_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
