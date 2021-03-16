
#include "hash.h"

// Adapted from http://cr.yp.to/cdb/cdb.txt

uint32_t cdb_hash(const void * data, uint32_t len) {
	const uint8_t * bytes = (const uint8_t *) data;
    uint32_t hash = 5381;

	while (len) {
		hash = hash * 33 ^ *bytes;
		len--;
		bytes++;
    }

    return hash;
}
