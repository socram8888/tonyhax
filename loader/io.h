
#pragma once
#include <stdint.h>

volatile uint32_t * const I_STAT = (volatile uint32_t *) 0x1F801070;
volatile uint32_t * const I_MASK = (volatile uint32_t *) 0x1F801074;
