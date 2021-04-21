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

#define MAXBUF 1024
#define DEBUG_FLAG 1

void handleClient(int clientSocket);
void recvFromClient(int clientSocket, char *buf);
int checkArgs(int argc, char *argv[]);
int checkForExit(char *buf);
void sendToClient(int clientSocket, char *pdu1);

int main(int argc, char *argv[])
{
	int serverSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for a client socket
	int activeSocket = 0;   //socket descriptor for the currently active socket
	int portNumber = 0;
	int waitingSockets[15] = {-1}; // holds the socket numbers that have connected, but have not yet sent their handle (flag 1)
	int numWaiting = 0; // number of waiting sockets in list
	// TODO: implement waiting on sockets
	
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	serverSocket = tcpServerSetup(portNumber);

	// set up poll
	setupPollSet();
	addToPollSet(serverSocket);

	// wait for client(s) to connect and recieve data
	while (1) {
		activeSocket = pollCall(-1);

		if (activeSocket == serverSocket) {
			// a client is trying to connect
			clientSocket = tcpAccept(serverSocket, DEBUG_FLAG);
			addToPollSet(clientSocket);
		}

		else {
			// a client is sending a PDU
			handleClient(activeSocket);
		}
	}
	
	/* close the sockets */
	// TODO: close all clients
	// Do I need to close any sockets? server only dies w ^C anyway
	close(serverSocket);
	
	return 0;
}

void handleClient(int clientSocket) {
	char pdu[MAXBUF];
	recvFromClient(clientSocket, pdu);
	if (checkForExit(pdu)) {
		// client is attempting to exit
		printf("Client %d exited\n", clientSocket);
		removeFromPollSet(clientSocket);
		close(clientSocket);
	}
	else {
		// Test the client's recv process
		sendToClient(clientSocket, pdu);
	}
}

void recvFromClient(int clientSocket, char *buf)
{
	uint16_t messageLen;
	recvPacket(clientSocket, buf);

	memcpy(&messageLen, buf, sizeof(messageLen));
	messageLen = ntohs(messageLen);

	printf("PDU Len: %d Message: %s\n", messageLen, buf + sizeof(uint16_t));
}

void sendToClient(int clientSocket, char *pdu1) {
	// Tests the clients receive process
	uint16_t pdu1Len;
	char pdu2Message[MAXBUF];
	memcpy(&pdu1Len, pdu1, sizeof(uint16_t));
	pdu1Len = ntohs(pdu1Len);

	sprintf(pdu2Message, "Number of bytes received by server was: %d", (int)pdu1Len);
	uint16_t pdu2Len = strnlen(pdu2Message, MAXBUF) + 1; // +1 for null termination

	sendPacket(clientSocket, pdu1 + 2, pdu1Len - 2); // factor of 2 removes the header
	sendPacket(clientSocket, pdu2Message, pdu2Len);
}

int checkForExit(char *buf) {
	/* checks if the given pdu indicates a client is exiting
	 * true for exit found, false for no exit found */
	uint16_t pduLen;
	memcpy(&pduLen, buf, sizeof(pduLen));
	pduLen = ntohs(pduLen);

	if (pduLen == 7) {
		// check for exit message in pdu
		return exitFound(buf + sizeof(uint16_t), pduLen - sizeof(uint16_t));
	}
	else if (pduLen > 0) {
		return 0;
	}
	// connection was closed
	return 1;
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

