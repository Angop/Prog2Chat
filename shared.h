/* Angela Kerlin
 * CPE464-01
 * contains shared code between server and cclient */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#define HEADER_BYTES 3
#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAXBUF 1024


int exitFound(char *buf, int len);