
#include "bios.h"

bool bios_is_european(void) {
	return (*((const char *) 0xBFC7FF52) == 'E');
}
