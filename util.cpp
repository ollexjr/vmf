#include "util.h"

#include <stdarg.h>
#include <iostream>

void Iprintf(int indent, char * format, ...) {
	//char buffer[256];
	//vsnprintf(buffer, 255, format, args);

	va_list args;
	va_start(args, format);
	for (int i = 1; i < indent; i++) {
		putchar('\t');
	}
	vprintf(format, args);
	va_end(args);
}
