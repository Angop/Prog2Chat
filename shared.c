/* Angela Kerlin
 * CPE464-01
 * contains shared code between server and cclient */

#include "shared.h"


int exitFound(char *buf, int len) {
	// checks if given string is "exit"
	if (len != 5) {
		return 0;
	}
	return !strncmp("exit\0", buf, 5);

}


uint8_t parseFlag(char *buf) {
    return buf[HEADER_BYTES-1];
}