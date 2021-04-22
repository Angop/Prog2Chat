/* Angela Kerlin
 * CPE464-01
 * Header for send and recieve functions
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "shared.h"


void sendPacket(int socketNum, char *sendData, int dataLen, uint8_t flag);
uint8_t recvPacket(int clientSocket, char *buf);