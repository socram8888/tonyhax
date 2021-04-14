
#include "integrity.h"
#include "crc.h"
#include <stdint.h>

// Loading address of tonyhax, provided by the secondary.ld linker script
extern uint8_t __RO_START__, __CRC_START__;

// True if integrity check succeeded
bool integrity_ok = false;

// Correct CRC value, which will be inserted after compilation
uint32_t __attribute__((section(".crc"))) integrity_correct_crc = 0xDEADBEEF;

void integrity_test() {
	uint32_t calc_value = crc32(&__RO_START__, &__CRC_START__ - &__RO_START__);
	integrity_ok = calc_value == integrity_correct_crc;
}
