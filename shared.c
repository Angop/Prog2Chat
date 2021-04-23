/* Angela Kerlin
 * CPE464-01
 * contains shared code between server and cclient */

#include "shared.h"



uint8_t parseFlag(char *buf) {
    return buf[HEADER_BYTES-1];
}