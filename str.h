
#pragma once
#include <stdarg.h>
#include <stdint.h>

int mini_vsprintf(char * str, const char * format, va_list args);

int memcmp(const void * ptr1, const void * ptr2, uint32_t num);

void * memmem(const void * haystack, uint32_t haystacklen, const void * needle, uint32_t needlelen);
