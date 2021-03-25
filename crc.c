
#include "crc.h"

// Adapted from https://stackoverflow.com/a/15031244/4454028

uint32_t crc32(const void * data, uint32_t len) {
	const uint8_t * bytes = (const uint8_t *) data;
	uint32_t crc = 0xFFFFFFFF;

    while (len) {
        crc ^= *bytes;

        for (int k = 0; k < 8; k++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ 0xEDB88320;
			} else {
				crc = (crc >> 1);
			}
        }

		bytes++;
		len--;
    }

    return ~crc;
}
