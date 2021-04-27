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
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "pollLib.h"
#include "sendrecv.h"
#include "shared.h"


void sendLoop(int socketNum);
int getIncoming(int socketNum, int wait);
void processIncoming(char *inBuf, uint16_t inBufLen, int serverSocket);
void incomingMessage(char *inBuf, uint16_t inBufLen);
void incomingList(char *inBuf, uint16_t inBufLen);
void listPrintHandles(uint32_t numHandles);
int getMessagePtr(char *inBuf, uint16_t inBufLen, char **message);
uint16_t recvFromServer(int socketNum, char *recvBuf);
int getCommand(char *buf, int len);
void handleCommand(char command, char *sendBuf, uint16_t sendLen, int socketNum);
int sendToServer(int socketNum, char *sendBuf, uint16_t sendLen);
int readFromStdin(char * buffer);
void checkArgs(int argc, char * argv[]);

void setupHandle(int socketNum, int argc, char **argv);
void commandExit(int socketNum);
void commandMessage(int sockNum, char *buf, int sendLen);
int getNumHandles(char *buf, int bufLen, char **bufOffset);
uint16_t copyHandles(char *sendBuf, char **buf, int numHandles, int bufLen);
uint16_t copyHandle(char *sendBuf, char **buf, int bufLen);
uint16_t copyMessage(char *sendBuf, char *buf, int numHandles, int bufLen);

// Global client variables
char clientHandle[MAX_HANDLE_LEN + 1];
uint8_t clientHandleLen = 0;

int main(int argc, char * argv[]) {
	int socketNum = 0;         //socket descriptor of server
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);
	setupHandle(socketNum, argc, argv);
	setupPollSet();
	addToPollSet(socketNum);
	addToPollSet(STDIN_FILENO);
	
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

	// format payload of send
	buf[0] = handleLen;
	memcpy(buf + 1, handle, handleLen);

	// set global handle name/len
	memcpy(clientHandle, handle, handleLen);
	clientHandleLen = handleLen;

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

void sendLoop(int serverSocket) {
	char buf [MAXBUF]; //data buffer
	uint16_t bufLen = 0; // length of buf
	char command = '\0';
	int socketNum = 0;


	do {
		printf("$: ");
		fflush(stdout);
		// get the active socket
		socketNum = getIncoming(socketNum, INDEF_POLL);
		
		// handle client input
		if (socketNum == STDIN_FILENO) {
			bufLen = readFromStdin(buf);
			command = getCommand(buf, bufLen);
			handleCommand(command, buf, bufLen, serverSocket);
		}

		// handle server input
		else {
			bufLen = recvFromServer(serverSocket, buf);
			processIncoming(buf, bufLen, serverSocket);
		}
	} while(command != 'e');
}

int getCommand(char *buf, int len) {
	// gets the given command, or null if none given
	if (len < 3 || (len >= 1 && buf[0] != '%')) {
		// no command given
		return -1;
	}
	return tolower(buf[1]);
}

void handleCommand(char command, char *sendBuf, uint16_t sendLen, int socketNum) {
	// fulfill given command
	// TODO
	if (command == 'e') {
		commandExit(socketNum);
	}
	else if (command == 'm') {
		commandMessage(socketNum, sendBuf, sendLen);
	}
	else if (command == 'l') {
		// requests a list of handles from server
		sendPacket(socketNum, NULL, 0, LST_REQ_FLAG);
	}
	else {
		// For now, just forward the nonsense to the server
		sendPacket(socketNum, sendBuf, sendLen, DEBUG_FLAG);
	}
}

int sendToServer(int socketNum, char *sendBuf, uint16_t sendLen) {
	sendPacket(socketNum, sendBuf, sendLen, DEBUG_FLAG);
	return sendLen;
}

int readFromStdin(char * buffer) {
	int inputLen = 0;        
	inputLen = read(STDIN_FILENO, buffer, MAXBUF - 1);

	// Null terminate the string
	buffer[inputLen - 1] = '\0'; // replaces newline
	
	return inputLen;

	// char aChar = 0;
	
	// // Important you don't input more characters than you have space 
	// buffer[0] = '\0';
	// while (inputLen < (MAXBUF - 1) && aChar != '\n')
	// {
	// 	aChar = getchar();
	// 	if (aChar != '\n')
	// 	{
	// 		buffer[inputLen] = aChar;
	// 		inputLen++;
	// 	}
	// }
	
	// // Null terminate the string
	// buffer[inputLen] = '\0';
	// inputLen++;
	
	// return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s host-name port-number handle \n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

int getIncoming(int socketNum, int wait) {
	// checks server for any incoming packets and deals with them
	//TODO
	char buf[MAXBUF];
	int result = 0;
	uint16_t pduLen = 0;
	if ((result=pollCall(wait)) == -1) {
		// server did not send anything
		// printf("Poll result: %d\n", result); //dd
		// return DEBUG_FLAG;
	}
	// printf("Poll result: %d\n", result); //dd
	return result;
}

void processIncoming(char *inBuf, uint16_t inBufLen, int serverSocket) {
	// TODO
	int flag = parseFlag(inBuf);

	if (flag == MSG_FLAG) {
		incomingMessage(inBuf, inBufLen);
	}
	else if (flag == LST_NUM_FLAG) {
		incomingList(inBuf, inBufLen);
	}
	else if (flag == EXIT_ACK_FLAG) {
		// Time for the client to exit
		close(serverSocket);
		exit(EXIT_SUCCESS);
	}
	else {
		// just print what is recieved
		printAsHex(inBuf, inBufLen);
	}
}

void incomingMessage(char *inBuf, uint16_t inBufLen) {
	uint8_t sendHandleLen = inBuf[HEADER_BYTES];
	char *sendHandle = inBuf + HEADER_BYTES + 1; // contains the start of handle
	char *message; // contains the start of handle
	int messageLen = getMessagePtr(inBuf, inBufLen, &message);

	printf("\n%.*s: %.*s\n", sendHandleLen, sendHandle, messageLen, message);
}

int getMessagePtr(char *inBuf, uint16_t inBufLen, char **message) {
	// TODO
	uint8_t offset = inBuf[HEADER_BYTES] + HEADER_BYTES + 2; // start of the first destination handle len
	uint8_t numHandles = inBuf[(inBuf[HEADER_BYTES])+ 1 + HEADER_BYTES];
	int i;
	int handleLen = 0;
	for (i=0; i < numHandles; i++) {
		handleLen = inBuf[offset];
		offset += handleLen;
	}
	*message = inBuf + offset + 1; // set the message pointer to begining of message

	return inBufLen - (*message - inBuf); // return the length of the message
}

void incomingList(char *inBuf, uint16_t inBufLen) {
	uint32_t numHandles = 0;
	memcpy(&numHandles, inBuf + HEADER_BYTES, sizeof(numHandles));
	numHandles = ntohl(numHandles);
	printf("Number of clients: %d\n", numHandles);

	listPrintHandles(numHandles);

	// Get the done message
	char buf[MAXBUF];
	int socketNum = pollCall(INDEF_POLL);
	recvFromServer(socketNum, buf);
}

void listPrintHandles(uint32_t numHandles) {
	int i = 0;
	int socketNum = 0;
	char hBuf[MAXBUF];
	uint8_t handleLen = 0;

	while (i < numHandles) {
		// poll server
		// TODO: will need to update this to ignore stdin socket
		socketNum = pollCall(INDEF_POLL); //dd I dont like this
		recvFromServer(socketNum, hBuf);
		handleLen = hBuf[HEADER_BYTES];
		printf("\t%.*s\n", handleLen, hBuf + HEADER_BYTES + 1);
		i++;
	}
}

uint16_t recvFromServer(int socketNum, char *recvBuf) {
	// receives a message from the server
	uint16_t msgLen = 0;
	uint8_t flag = 0;

	recvPacket(socketNum, recvBuf);
	memcpy(&msgLen, recvBuf, sizeof(uint16_t));
	memcpy(&flag, recvBuf + sizeof(uint16_t), sizeof(uint8_t));
	msgLen = ntohs(msgLen);

	// printf("Recv() from Server bytes-%d and flag-%d: ", msgLen, flag); //dd 
	// printf("%.*s\n", (int)(msgLen - HEADER_BYTES), recvBuf + HEADER_BYTES);//dd 
	return msgLen;
}

void commandExit(int socketNum) {
	// send exit pdu
	sendPacket(socketNum, NULL, 0, EXIT_FLAG);
}

void commandMessage(int sockNum, char *buf, int bufLen) {
	// given the client input, format and send message to server
	char sendBuf[MAXBUF]; // holds the payload of intended message
	char *bufOffset = NULL; // functions use this to coordinate reading of buf

	// format payload
	memcpy(sendBuf, &clientHandleLen, sizeof(uint8_t)); // sending handle len
	printf("\tCHandle len: %d\n", clientHandleLen); //dd

	memcpy(sendBuf + sizeof(uint8_t), clientHandle, clientHandleLen); // sending handle (no null term)

	printf("\tCHandle: %s\n", clientHandle); //dd
	int numHandles = getNumHandles(buf, bufLen, &bufOffset);
	if (numHandles == -1) {
		printf("Invalid command format\n");
		return;
	}
	memcpy(sendBuf + clientHandleLen + 1, &numHandles, sizeof(uint8_t)); // number of destination handles
	printf("\tNumHandles: %d\n", numHandles); //dd
	uint16_t sendLen = sizeof(clientHandleLen) + clientHandleLen + sizeof(uint8_t);
	sendLen += copyHandles(sendBuf + sendLen, &bufOffset, numHandles, bufLen - (bufOffset - buf)); // each handle's length and the handle
	sendLen += copyMessage(sendBuf + sendLen, bufOffset, numHandles, bufLen - (bufOffset - buf));  // text message

	printf("\tsendLen: %d\n", sendLen); //dd
	printAsHex(sendBuf, sendLen); // dd
	sendPacket(sockNum, sendBuf, sendLen, MSG_FLAG);
}

int getNumHandles(char *buf, int bufLen, char **bufOffset) {
	// TODO: test
	int numHandles = -1;
	if (bufLen >= 3) {
		// can have a valid number
		numHandles=strtol(buf + 2, bufOffset, 10); // skip the %m
	}
	if (bufLen < 3 || *bufOffset == buf || numHandles <= 0) {
		// no number is provided
		return -1;
	}
	return numHandles;
}

uint16_t copyHandles(char *sendBuf, char **buf, int numHandles, int bufLen) {
	// given the number of handles, copies handle into the send buf
	// buf pointer must be at the start of the FIRST handle provided (or whitespace BEFORE it)
	int i;
	int offset = 0;

	for (i=0; i < numHandles; i++) {
		// printf("\thandle %d: ", i); //dd
		offset += copyHandle(sendBuf + offset, buf, bufLen - offset);
		// printf("\n"); //dd
	}
	return offset; // offset is for sendBuf
}

uint16_t copyHandle(char *sendBuf, char **buf, int bufLen) {
	// remove white space before first handle
	uint8_t i = 0;
	uint8_t whiteSpace = 0;
	char c = (*buf)[i];
	while (i < bufLen && isspace(c)) {
		c = (*buf)[i];
		i++;
	}
	whiteSpace = i - 1;
	if (whiteSpace <= 0) { i++;	}
	// copy each handle and its len into the send buf
	while (i < bufLen && !isspace(c)) {
		// printf("%c",c); //dd
		sendBuf[sizeof(uint8_t) + i - whiteSpace - 1] = c;
		c = (*buf)[i];
		i++;
	}
	sendBuf[0] = i - whiteSpace - 1; // number of bytes not including preceeding white space // dd idk if -1 is right
	*buf += i; // increment buf by the number of bytes in the handle
	return i - whiteSpace;
}

uint16_t copyMessage(char *sendBuf, char *buf, int numHandles, int bufLen) {
	// copies the message in buf to sendBuf, ignoring preceeding white space
	// remove white space before the message
	uint8_t i = 0;
	uint8_t whiteSpace = 0;
	char c = buf[i];
	while (i < bufLen && isspace(c)) {
		c = buf[i];
		i++;
	}
	whiteSpace = i;
	if (whiteSpace == 0) { i++;	}

	// copy over the message until the end of string
	// printf("\tmessage: "); //dd
	while (i < bufLen) {
		// printf("%c",c); //dd
		sendBuf[i - whiteSpace - 1] = c;
		c = buf[i];
		i++;
	}
	// printf("\n"); //dd
	return i - whiteSpace;
}
