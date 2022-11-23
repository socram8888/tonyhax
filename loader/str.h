
#pragma once
#include <stdarg.h>
#include <stddef.h>

void bzero(void * start, size_t len);

void memcpy(void * dest, const void * src, size_t len);

int isspace(int c);

int mini_sprintf(char * str, const char * format, ...);

int mini_vsprintf(char * str, const char * format, va_list args);

int strlen(const char * str);

char * strchr(const char * str, int c);

char * strcpy(char * dest, const char * src);

int strcmp(const char * a, const char * b);

int strncmp (const char * a, const char * b, size_t len);
