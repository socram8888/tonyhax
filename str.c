
#include "str.h"
#include <stddef.h>

int mini_vsprintf(char * str, const char * format, va_list args) {
	char * pos = str;

	while (*format != 0) {
		switch (*format) {
			case '\\':
				format++;

				switch (*format) {
					case 'n':
						*pos = '\n';
						pos++;
						break;

					case 'r':
						*pos = '\r';
						pos++;
						break;

					case '\0':
						*pos = '\\';
						pos++;
						goto end;

					default:
						pos[0] = '\\';
						pos[1] = *format;
						pos += 2;
						break;
				}

				break;

			case '%':
				format++;

				switch (*format) {
					case 's': {
						const char * subtext = va_arg(args, const char *);
						while (*subtext != '\0') {
							*pos = *subtext;
							pos++;
							subtext++;
						}
						break;
					}

					case 'x':
					case 'X': {
						char c;
						uint32_t val = va_arg(args, uint32_t);
						for (int i = 28; i >= 0; i -= 4) {
							c = (val >> i) & 0xF;
							if (c < 10) {
								c += '0';
							} else {
								c += 'A' - 0xA;
							}
							*pos = c;
							pos++;
						}
						break;
					}

					case '\0':
						*pos = '\\';
						pos++;
						goto end;

					default:
						pos[0] = '%';
						pos[1] = *format;
						pos += 2;
						break;
				}
				break;

			default:
				*pos = *format;
				pos++;
				break;
		}

		format++;
	}

end:
	*pos = '\0';

	return pos - str;
}

int memcmp(const void * ptr1, const void * ptr2, uint32_t num) {
	const uint8_t * bytes1 = (const uint8_t *) ptr1;
	const uint8_t * bytes2 = (const uint8_t *) ptr2;
	int diff = 0;

	for (uint32_t i = 0; i < num && diff == 0; i++) {
		diff = bytes1[i] - bytes2[i];
	}

	return diff;
}

void * memmem(const void * haystack, uint32_t haystacklen, const void * needle, uint32_t needlelen) {
	const uint8_t * haystackbytes = (const uint8_t *) haystack;
	const uint8_t * needlebytes = (const uint8_t *) needle;

	while (haystacklen >= needlelen) {
		if (memcmp(haystackbytes, needlebytes, needlelen) == 0) {
			return (void *) haystackbytes;
		}
		haystacklen--;
		haystackbytes++;
	}

	return NULL;
}
