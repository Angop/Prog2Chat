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
void handleAccept(int serverSocket);
void handleClient(int clientSocket);
void recvFromClient(int clientSocket, char *buf);
int checkArgs(int argc, char *argv[]);
int checkForExit(char *buf);
void forwardToClient(int clientSocket, char *pdu1); // forwards existing pdu to client

// functions to handle each of the user functions
void initialPacket(char *pdu, int socketNum);
void clientExit(int clientSocket);

int main(int argc, char *argv[]) {
	int serverSocket = setupServer(argc, argv);   //socket descriptor for the server socket
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
	
	return 0;
}

void handleAccept(int serverSocket) {
	int clientSocket = tcpAccept(serverSocket, DEBUG_FLAG);
	addToPollSet(clientSocket);
	// TODO: handle waiting list of clients who have connected but not yet sent initial packet
}

void handleClient(int clientSocket) {
	char pdu[MAXBUF];
	recvFromClient(clientSocket, pdu);
	uint8_t flag = parseFlag(pdu);

	printf("YOURE GOOD1\n");
	if (flag == EXIT_FLAG || checkForExit(pdu)) {
		// client is attempting to exit or they ^C
		printf("YOURE GOOD2\n");
		clientExit(clientSocket);
	}
	else if (flag == INIT_FLAG) {
		printf("YOURE NOT GOOD3\n");
		initialPacket(pdu, clientSocket);
	}
}

// high level client functionality

void initialPacket(char *pdu, int socketNum) {
	// response to flag=1
	uint8_t handleLen = 0; // length of handle not including null byte
	char handle[handleLen + 1]; // +1 for null
	handle[handleLen] = '\0'; // null terminate

	// TODO: error check bad handleLen? 3+1+handleLen==pduLen?
	memcpy(&handleLen, pdu + HEADER_BYTES, sizeof(uint8_t));
	memcpy(handle, pdu + HEADER_BYTES + 1, handleLen);

	if (!addSocketHandle(socketNum, handle, 1)) {
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
	printf("Client %d exited\n", clientSocket);
	if (!closeSocketHandle(clientSocket)) {
		fprintf(stderr, "closeSocketHandle: socket does not exist");
		exit(EXIT_FAILURE);
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

void forwardToClient(int clientSocket, char *pdu1) {
	// Tests the clients receive process
	uint16_t pdu1Len;
	char pdu2Message[MAXBUF];
	memcpy(&pdu1Len, pdu1, sizeof(uint16_t));
	pdu1Len = ntohs(pdu1Len);

	sprintf(pdu2Message, "Number of bytes received by server was: %d", (int)pdu1Len);
	uint16_t pdu2Len = strnlen(pdu2Message, MAXBUF) + 1; // +1 for null termination

	sendPacket(clientSocket, pdu1 + HEADER_BYTES, pdu1Len - HEADER_BYTES, 14);
	sendPacket(clientSocket, pdu2Message, pdu2Len, 14);
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
