/******************************************************************************
* Angela Kerlin
* myClient.c
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "sendrecv.h"
#include "shared.h"


int sendToServer(int socketNum, char *sendBuf);
void sendLoop(int socketNum);
int readFromStdin(char * buffer);
void checkArgs(int argc, char * argv[]);
void recvFromServer(int socketNum);
void setupHandle(int socketNum, int argc, char **argv);

int main(int argc, char * argv[]) {
	int socketNum = 0;         //socket descriptor of server
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);
	setupHandle(socketNum, argc, argv);
	
	sendLoop(socketNum);
	
	close(socketNum);
	
	return 0;
}

void setupHandle(int socketNum, int argc, char **argv) {
	// TODO: verify valid handle given
	char buf[MAXBUF] = {0};
	char *handle = argv[3];
	uint8_t handleLen = strnlen(handle, MAX_HANDLE_LEN);
	int flag = 0;
	printf("handleLenClient: %d\n", handleLen);

	// format payload of send
	buf[0] = handleLen;
	memcpy(buf + 1, handle, handleLen);

	sendPacket(socketNum, buf, handleLen + 1, INIT_FLAG); // + 1 for handleLen
	recvPacket(socketNum, buf); // receive response to setup
	if ((flag=parseFlag(buf)) == INIT_ERR_FLAG) {
		// handle is taken
		printf("Error on initial packet (handle already exists)\n");
		close(socketNum);
		exit(EXIT_FAILURE);
	}
	else if (flag != INIT_ACPT_FLAG) {
		// debug bad server response
		printf("Error on initial packet (server gave bad response)\n");
		exit(EXIT_FAILURE);
	}
	// otherwise, the handle is good!
	printf("Connection successful!\n");
}

void sendLoop(int socketNum) {
	char sendBuf[MAXBUF]; //data buffer
	int sendLen = 0; //data buffer
	do {
		sendLen = sendToServer(socketNum, sendBuf);
	} while(!exitFound(sendBuf, sendLen));
}

int sendToServer(int socketNum, char *sendBuf) {
	uint16_t sendLen = 0; //amount of data to send
	
	sendLen = readFromStdin(sendBuf);
	printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
	
	sendPacket(socketNum, sendBuf, sendLen, 14); // TODO: temp placeholder 14 for flag
	return sendLen;
}

int readFromStdin(char * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	printf("Enter data: ");
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s host-name port-number handle \n", argv[0]);
		exit(1);
	}
}

void recvFromServer(int socketNum) {
	// receives a message from the server
	char recvBuf[MAXBUF];
	uint16_t msgLen = 0;
	uint8_t flag = 0;

	recvPacket(socketNum, recvBuf);
	memcpy(&msgLen, recvBuf, sizeof(uint16_t));
	memcpy(&flag, recvBuf + sizeof(uint16_t), sizeof(uint8_t));
	msgLen = ntohs(msgLen);


	printf("Recv() from Server bytes-%d and flag-%d: ", msgLen, flag);
	printf("%.*s\n", (int)(msgLen - HEADER_BYTES), recvBuf + HEADER_BYTES);
}
