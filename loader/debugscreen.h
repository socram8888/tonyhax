
#pragma once
#include <stdint.h>
#include <stdbool.h>

void debug_init();

void debug_write(const char * str, ...);

void debug_text_at(uint_fast16_t x, uint_fast16_t y, const char * str);

void debug_switch_standard(bool pal);

extern bool controller_input;
