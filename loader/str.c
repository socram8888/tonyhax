
#include "str.h"
#include <stdint.h>

int isspace(int c) {
	switch (c) {
		case ' ':
		case '\r':
		case '\n':
		case '\t':
		case '\v':
		case '\f':
			return 1;

		default:
			return 0;
	}
}

int mini_sprintf(char * str, const char * format, ...) {
	va_list args;
	va_start(args, format);
	return mini_vsprintf(str, format, args);
}

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
					case 'c': {
						int c = va_arg(args, int);
						*pos = c;
						pos++;
						break;
					}

					case 's': {
						const char * subtext = va_arg(args, const char *);
						while (*subtext != '\0') {
							*pos = *subtext;
							pos++;
							subtext++;
						}
						break;
					}

					case 'd': {
						int val = va_arg(args, int);

						if (val == 0) {
							*pos = '0';
							pos++;
						} else {
							if (val < 0) {
								*pos = '-';
								pos++;
							}

							// Calculate digits backwards
							char digits[10];
							int digpos = 0;
							do {
								digits[digpos] = '0' + val % 10;
								val = val / 10;
								digpos++;
							} while (val != 0);

							// And copy them now also backwards
							while (digpos != 0) {
								digpos--;
								*pos = digits[digpos];
								pos++;
							}
						}
						break;
					}

					case 'x': {
						char c;
						unsigned int val = va_arg(args, unsigned int);
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

// The BIOS has already this function but for clearing the console's RAM it's unbearably slow.
void bzero(void * start, size_t len) {
	uint8_t * bytes = (uint8_t *) start;
	while (len) {
		*bytes = 0;
		bytes++;
		len--;
	}
}
