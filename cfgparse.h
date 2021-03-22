
#pragma once
#include <stdint.h>
#include <stdbool.h>

bool config_get_hex(const char * config, const char * wanted, uint32_t * value);

bool config_get_string(const char * config, const char * wanted, char * value);
