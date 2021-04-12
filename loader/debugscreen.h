
#pragma once
#include <stdint.h>

void debug_init();

void debug_write(const char * str, ...);

void debug_text_at(uint_fast16_t x, uint_fast16_t y, const char * str);
