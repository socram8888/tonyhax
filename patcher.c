
#include <stddef.h>
#include <string.h>
#include "patcher.h"
#include "debugscreen.h"
#include "hash.h"

// Last patch in list
#define FLAG_LAST 0x01

// Null the bytes
#define FLAG_NOP  0x02

struct patch {
	uint32_t offset;
	uint16_t size;
	uint8_t flags;
	const uint8_t * data;
};

struct game {
	const char * name;
	uint32_t hash;
	const struct patch * patches;
};

const struct game GAMES[] = {
	{
		.name = "Tomba 2 (US)",
		.hash = 0x4122E9DC,
		.patches = (struct patch[]) {
			{
				// Nuke the call to the check antipiracy function
				.offset = 0x8001127C,
				.size = 4,
				.flags = FLAG_LAST | FLAG_NOP,
			}
		}
	}
};

const struct game * find_game(uint32_t hash) {
	for (uint32_t i = 0; i < sizeof(GAMES) / sizeof(GAMES[0]); i++) {
		if (GAMES[i].hash == hash) {
			return GAMES + i;
		}
	}

	return NULL;
}

void patch_game(const uint8_t * header) {
	uint32_t load_offset = *((const uint32_t *) (header + 0x08));
	uint32_t exec_size   = *((const uint32_t *) (header + 0x0C));

	uint32_t exec_hash = cdb_hash((uint8_t *) load_offset, exec_size);
	debug_write("Exec hash: %x", exec_hash);

	const struct game * game = find_game(exec_hash);
	if (game == NULL) {
		debug_write("Not in database");
		return;
	}

	debug_write("Detected: %s", game->name);

	const struct patch * cur_patch = game->patches;
	while (1) {
		debug_write(" - Patch %x", cur_patch->offset);

		uint8_t * patch_dest = (uint8_t *) cur_patch->offset;
		if (cur_patch->flags & FLAG_NOP) {
			memset(patch_dest, 0, cur_patch->size);
		} else {
			memcpy(patch_dest, cur_patch->data, cur_patch->size);
		}

		if (cur_patch->flags & FLAG_LAST) {
			break;
		}

		cur_patch++;
	}
}
