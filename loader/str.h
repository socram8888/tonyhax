
#pragma once
#include <stdarg.h>
#include <stdint.h>

int isspace(int c);

int mini_sprintf(char * str, const char * format, ...);

int mini_vsprintf(char * str, const char * format, va_list args);
