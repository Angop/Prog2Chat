/* Angela Kerlin
 * CPE464-01
 * contains shared code between server and cclient */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#define HEADER_BYTES 3
#define MAXBUF 1024
#define DEBUG_FLAG 14
#define MAXBUF 1024
#define MAX_HANDLE_LEN 100
#define IMM_POLL 0
#define INDEF_POLL -1

#define INIT_FLAG 1
#define INIT_ACPT_FLAG 2
#define INIT_ERR_FLAG 3
#define BRC_FLAG 4
#define MSG_FLAG 5
#define EXIT_FLAG 8
#define EXIT_ACK_FLAG 9
#define LST_REQ_FLAG 10
#define LST_NUM_FLAG 11
#define LST_HAN_FLAG 12
#define LST_DNE_FLAG 13

int exitFound(char *buf, int len);
uint8_t parseFlag(char *buf);
void printAsHex(char *buf, uint16_t len);