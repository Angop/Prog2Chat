/******************************************************************************
* Angela Kerlin
* tcp_server.c
*
* CPE 464 - Program 1
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
#include "pollLib.h"
#include "shared.h"
#include "socketHandle.h"

int setupServer(int argc, char *argv[]);
int mainLoop(int serverSocket);
void handleAccept(int serverSocket);
void handleClient(int clientSocket);
void recvFromClient(int clientSocket, char *buf);
int checkArgs(int argc, char *argv[]);
int checkForExit(char *buf);
void forwardToClient(int clientSocket, char *orig, int flag); // forwards existing pdu to client

// functions to handle each of the user functions
void initialPacket(char *pdu, int socketNum);
void clientExit(int clientSocket);
void messageFlag(char *pdu, int socketNum);
void listFlag(int socketNum);
void broadcastFlag(char *pdu, int socketNum);

// higher level helpers
uint8_t getNumHandles(char *pdu, uint16_t pduLen);
void getSockets(int numHandles, char *pdu, int *destSockets, int sendSocket);
void badDestHandle(int sendSocket, char *handle, int handleLen);
void listSendHandles(uint32_t numHandles, char (*handleList)[101], int socketNum);
void bcSendHandles(char *pdu, uint32_t entries, int *socketList, int senderSocket);

int main(int argc, char *argv[]) {
	int serverSocket = setupServer(argc, argv);   //socket descriptor for the server socket
	mainLoop(serverSocket);
	return 0;
}

int mainLoop(int serverSocket) {
	int activeSocket = 0;   //socket descriptor for the currently active socket
	// wait for client(s) to connect and recieve data
	while (1) {
		activeSocket = pollCall(-1);
		if (activeSocket == serverSocket) {
			// a client is trying to connect
			handleAccept(serverSocket);
		}
		else {
			// a client is sending a PDU
			handleClient(activeSocket);
		}
	}
}

void handleAccept(int serverSocket) {
	int clientSocket = tcpAccept(serverSocket, DEBUG_FLAG);
	addToPollSet(clientSocket);
}

void handleClient(int clientSocket) {
	char pdu[MAXBUF];
	recvFromClient(clientSocket, pdu);
	uint8_t flag = parseFlag(pdu);

	if (flag == EXIT_FLAG || checkForExit(pdu)) {
		// client is attempting to exit or they ^C
		clientExit(clientSocket);
	}
	else if (flag == INIT_FLAG) {
		initialPacket(pdu, clientSocket);
	}
	else if (flag == BRC_FLAG) {
		printf("BROADCAST\n");
		broadcastFlag(pdu, clientSocket);
	}
	else if (flag == MSG_FLAG) {
		messageFlag(pdu, clientSocket);
	}
	else if (flag == LST_REQ_FLAG) {
		listFlag(clientSocket);
	}
}

// high level client functionality

void initialPacket(char *pdu, int socketNum) {
	// response to flag=1
	uint8_t handleLen = 0; // length of handle not including null byte
	char handle[handleLen + 1]; // +1 for null

	memcpy(&handleLen, pdu + HEADER_BYTES, sizeof(uint8_t));
	memcpy(handle, pdu + HEADER_BYTES + 1, handleLen);
	handle[handleLen] = '\0'; // null terminate


	if (!addSocketHandle(socketNum, handle, handleLen)) {
		// duplicate handle
		removeFromPollSet(socketNum);
		sendPacket(socketNum, NULL, 0, INIT_ERR_FLAG);
		printf("Denied connection on handle: %s\n", handle);
		close(socketNum);
	}
	else {
		sendPacket(socketNum, NULL, 0, INIT_ACPT_FLAG);
		printf("Handle len: %d recv: %.*s\n",handleLen, handleLen,handle);
	}
}

void clientExit(int clientSocket) {
	// send exit acknowledgment
	sendPacket(clientSocket, NULL, 0, EXIT_ACK_FLAG);
	printf("Client exited. Socket: %d\n", clientSocket);

	// clean up client
	if (!closeSocketHandle(clientSocket)) {
		fprintf(stderr, "closeSocketHandle: socket does not exist");
		exit(EXIT_FAILURE);
	}
}

void messageFlag(char *pdu, int socketNum) {
	// sends incoming message from client to destination client(s)
	// find the list of destination handles
	uint16_t pduLen;
	memcpy(&pduLen, pdu, sizeof(pduLen));
	pduLen = ntohs(pduLen);
	printf("PDULEN: %d", pduLen);
	uint8_t numHandles = getNumHandles(pdu, pduLen);
	int destSockets[numHandles];
	printAsHex(pdu, pduLen);
	getSockets(numHandles, pdu, destSockets, socketNum);
	printf("GOT SOCKETS\n");

	// send to each destination client
	uint8_t i;
	for (i=0; i < numHandles; i++) {
		if (destSockets[i] != -1) {
			forwardToClient(destSockets[i], pdu, MSG_FLAG);
		}
	}
}

uint8_t getNumHandles(char *pdu, uint16_t pduLen) {
	// gets the number of destination handles in a message pdu
	uint8_t handleLen = pdu[0 + HEADER_BYTES];

	return pdu[handleLen + 1 + HEADER_BYTES];
}
void getSockets(int numHandles, char *pdu, int *destSockets, int sendSocket) {
	uint8_t offset = pdu[HEADER_BYTES] + HEADER_BYTES + 2; // start of the first destination handle len
	int i = 0;
	char handle[MAX_HANDLE_LEN];
	uint8_t handleLen = 0;

	for (i=0; i < numHandles; i++) {
		// get the handle
		printf("OFFSET=%d\n",offset);
		handleLen = pdu[offset];
		printf("HANDLELEN=%d\n",handleLen);
		memcpy(handle, pdu + offset + 1, handleLen);

		// look up its socket number
		if ((destSockets[i] = getSocket(handle, handleLen)) == -1) {
			// handle does not exist
			badDestHandle(sendSocket, handle, handleLen);
		}
		offset += handleLen;
	}
}

void badDestHandle(int sendSocket, char *handle, int handleLen) {
	// tells sending client that destination handle does not exist
	char sendBuf[MAXBUF];
	sendBuf[0] = handleLen;
	memcpy(sendBuf + 1, handle, handleLen);

	sendPacket(sendSocket, sendBuf, handleLen + 1, MSG_DNE_FLAG);
}

void listFlag(int socketNum) {
	uint32_t entries = getNumEntries();
    char handleList[entries][101];
	getAllHandles(handleList);

	// send flag 11 with number of handles
	char sendBuf[MAXBUF];
	entries = htonl(entries);
	memcpy(sendBuf, &entries, sizeof(entries));
	sendPacket(socketNum, sendBuf, sizeof(entries), LST_NUM_FLAG);
	entries = ntohl(entries);

	// send each of the handles
	listSendHandles(entries, handleList, socketNum);
	
	// send done flag
	sendPacket(socketNum, NULL, 0, LST_DNE_FLAG);
}

void listSendHandles(uint32_t numHandles, char (*handleList)[101], int socketNum) {
	int i = 0;
	char sendBuf[MAXBUF];
	uint8_t handleLen = 0;

	for (i=0; i<numHandles; i++) {
		handleLen = strnlen(handleList[i], MAX_HANDLE_LEN);
		printf("%d\n",handleLen);
		memcpy(sendBuf, &handleLen, sizeof(handleLen));
		memcpy(sendBuf + sizeof(handleLen), handleList[i], handleLen);
		sendPacket(socketNum, sendBuf, handleLen + sizeof(handleLen), LST_HAN_FLAG);
	}
}

void broadcastFlag(char *pdu, int socketNum) {
	// sends message to all clients, except sending client
	uint32_t entries = getNumEntries();
    int socketList[entries];
	getAllSockets(socketList);

	uint8_t sendHandleLen = pdu[HEADER_BYTES];
	char sendHandle[pdu[HEADER_BYTES] + 1];
	memcpy(sendHandle, pdu + HEADER_BYTES + 1, sendHandleLen);
	sendHandle[sendHandleLen] = '\0';

	bcSendHandles(pdu, entries, socketList, socketNum);
}

void bcSendHandles(char *pdu, uint32_t entries, int *socketList, int senderSocket) {
	int i = 0;
	while (i < entries) {
		if (socketList[i] != senderSocket) {
			// send if its not the sender socket
			forwardToClient(socketList[i], pdu, BRC_FLAG);
		}
		i++;
	}
}

void recvFromClient(int clientSocket, char *buf)
{
	uint16_t messageLen;
	recvPacket(clientSocket, buf);

	memcpy(&messageLen, buf, sizeof(messageLen));
	messageLen = ntohs(messageLen);

	printf("PDU Len: %d Flag: %d Message: %.*s\n", messageLen, buf[2], messageLen - 3,buf + 3);
}

void forwardToClient(int clientSocket, char *orig, int flag) {
	// forwards an existing packet to specified client
	char newMessage[MAXBUF];
	uint16_t origLen;
	memcpy(&origLen, orig, sizeof(uint16_t));
	origLen = ntohs(origLen);

	sprintf(newMessage, "Number of bytes received by server was: %d", (int)origLen);
	// uint16_t newLen = strnlen(newMessage, MAXBUF) + 1; // +1 for null termination

	sendPacket(clientSocket, orig + HEADER_BYTES, origLen - HEADER_BYTES, flag);
	// sendPacket(clientSocket, newMessage, newLen, 14);
}


int checkForExit(char *buf) {
	/* checks if the given pdu indicates a client is exiting
	 * true for exit found, false for no exit found */
	uint16_t pduLen;
	memcpy(&pduLen, buf, sizeof(pduLen));
	pduLen = ntohs(pduLen);

	if (pduLen > 0) {
		return 0;
	}
	// connection was closed
	return 1;
}

int setupServer(int argc, char *argv[]) {
	//create the server socket
	int portNumber = 0;
	portNumber = checkArgs(argc, argv);
	int serverSocket = tcpServerSetup(portNumber);

	// set up poll
	setupPollSet();
	addToPollSet(serverSocket);
	return serverSocket;
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}
