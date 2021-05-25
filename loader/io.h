
#pragma once
#include <stdint.h>

#define _REG32(x) (*((volatile uint32_t *) x))
#define _REG16(x) (*((volatile uint16_t *) x))

#define I_STAT _REG32(0x1F801070)
#define I_MASK _REG32(0x1F801074)

#define INT_VBLANK (1 << 0)
#define INT_GPU (1 << 1)
#define INT_CDROM (1 << 2)
#define INT_DMA (1 << 3)
#define INT_TMR0 (1 << 4)
#define INT_TMR1 (1 << 5)
#define INT_TMR2 (1 << 6)
#define INT_JOYPAD (1 << 7)
#define INT_SIO (1 << 8)
#define INT_SPU (1 << 9)
#define INT_LIGHTGUN (1 << 10)

#define GPU_STAT _REG32(0x1F801814)
