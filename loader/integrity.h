
#pragma once
#include <stdbool.h>

/**
 * True if the integrity check succeeded.
 */
extern bool integrity_ok;

/**
 * Executes the integrity check and updates the integrity_ok variable.
 * Should be run before any change to the data section is made.
 */
void integrity_test(void);
