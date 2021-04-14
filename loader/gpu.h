
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define GPU_DISPLAY_H256 0
#define GPU_DISPLAY_H320 1
#define GPU_DISPLAY_H512 2
#define GPU_DISPLAY_H640 3

#define GPU_DISPLAY_INTERLACED (1 << 5)
#define GPU_DISPLAY_V240 (0 << 2)
#define GPU_DISPLAY_V480 (1 << 2 | GPU_DISPLAY_INTERLACED)

#define GPU_DISPLAY_NTSC (0 << 3)
#define GPU_DISPLAY_PAL  (1 << 3)

#define GPU_DISPLAY_15BPP (0 << 4)
#define GPU_DISPLAY_24BPP (1 << 4)

struct gpu_point {
	uint16_t x;
	uint16_t y;
};
typedef struct gpu_point gpu_point_t;

struct gpu_size {
	uint16_t width;
	uint16_t height;
};
typedef struct gpu_size gpu_size_t;

struct gpu_solid_rect {
	gpu_point_t pos;
	gpu_size_t size;
	uint32_t color;
	uint8_t semi_transp;
};
typedef struct gpu_solid_rect gpu_solid_rect_t;

struct gpu_tex_rect {
	gpu_point_t pos;
	gpu_size_t size;
	gpu_point_t texcoord;
	gpu_point_t clut;
	uint32_t color;
	uint8_t semi_transp;
	uint8_t raw_tex;
};
typedef struct gpu_tex_rect gpu_tex_rect_t;

bool gpu_is_pal(void);

void gpu_reset(void);

void gpu_wait_vblank(void);

void gpu_display_enable(void);

void gpu_display_disable(void);

void gpu_display_mode(uint32_t mode);

void gpu_copy_rectangle(const gpu_point_t * src, const gpu_point_t * dst, const gpu_size_t * size);

void gpu_draw_solid_rect(const gpu_solid_rect_t * rect);

void gpu_draw_tex_rect(const gpu_tex_rect_t * rect);

void gpu_set_drawing_area(const gpu_point_t * pos, const gpu_size_t * size);

void gpu_flush_cache(void);

void gpu_set_hrange(uint_fast16_t x1, uint_fast16_t x2);

void gpu_set_vrange(uint_fast16_t y1, uint_fast16_t y2);
