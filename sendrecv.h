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

#define MAXBUF 1024

void sendPacket(int socketNum, char *sendData, int dataLen);
void recvPacket(int clientSocket, char *buf);
int exitFound(char *buf, int len);