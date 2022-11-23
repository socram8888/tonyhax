
#include <stddef.h>
#include <stdbool.h>
#include "str.h"
#include "str.h"
#include "bios.h"
#include "debugscreen.h"

const char * find_wanted(const char * config, const char * wanted) {
	uint32_t wanted_len = strlen(wanted);

	// While first N characters don't match
	while (strncmp(config, wanted, wanted_len) != 0) {
		// No luck. Advance until next line.
		config = strchr(config, '\n');

		// If this is the last line, abort.
		if (config == NULL) {
			debug_write("Missing %s", wanted);
			return NULL;
		}

		// Advance to skip line feed.
		config++;
	}

	// Advance to after the name
	return config + wanted_len;
}

bool config_get_hex(const char * config, const char * wanted, uint32_t * value) {
	config = find_wanted(config, wanted);
	if (!config) {
		return false;
	}

	// Keep parsing until we hit the end of line or the end of file
	uint32_t parsed = 0;
	while (*config != '\n' && *config != '\0') {
		uint32_t digit = todigit(*config);
		if (digit < 0x10) {
			parsed = parsed << 4 | digit;
		}
		config++;
	}

	// Save and return
	*value = parsed;
	return true;
}

bool config_get_string(const char * config, const char * wanted, char * value) {
	config = find_wanted(config, wanted);
	if (!config) {
		return false;
	}

	// Advance until the start
	while (*config == '=' || isspace(*config)) {
		config++;
	}

	// Copy until space or end of file
	char * valuecur = value;
	while (*config != '\0' && !isspace(*config)) {
		*valuecur = *config;
		config++;
		valuecur++;
	}

	// Null terminate and return
	*valuecur = '\0';
	return true;
}
