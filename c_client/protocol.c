#include <stdlib.h>
#include <string.h>

int char_to_int(char c) {
	return c-0x30;
}

int protocol_authorize(const char * n) {

	return (atoi(n) * char_to_int(n[1])) + (char_to_int(n[3])*5);
}

