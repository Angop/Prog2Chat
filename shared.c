/* Angela Kerlin
 * CPE464-01
 * contains shared code between server and cclient */

#include "shared.h"



uint8_t parseFlag(char *buf) {
    return buf[HEADER_BYTES-1];
}

void printAsHex(char *buf, uint16_t len) {
	int i;
	for (i = 0; i < len; i++) {
		printf("%x ", buf[i]);
		if (!(i % 8)) {
			// every 8 bits print a tab
			printf("\t");
		}
		if (!(i % 24)) {
			// every 24 bits print a newline
			printf("\n");
		}
	}
	printf("\n\n");
	for (i = 0; i < len; i++) {
		printf("%c ", buf[i]);
		if (!((i + 1) % 8)) {
			// every 8 bits print a tab
			printf("\t");
		}
		if (!((i + 1) % 24)) {
			// every 24 bits print a newline
			printf("\n");
		}
	}
	printf("\n");
}