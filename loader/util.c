
#include "gpu.h"
#include "util.h"

void delay_ds(uint32_t deciseconds) {
	uint32_t frames = deciseconds * (gpu_is_pal() ? 5 : 6);
	while (frames) {
		gpu_wait_vblank();
		frames--;
	}
}
