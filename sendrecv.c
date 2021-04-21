/* Angela Kerlin
 * Handles sending and recieving for both the client and server
 */

#include "sendrecv.h"
int safeRecv(int socketNum, char *buf, uint16_t len);

void sendPacket(int socketNum, char *sendData, int dataLen) {
	// TODO: check for buf overflow (too long of a message)
	char sendBuf[MAXBUF];   // data buffer
	uint16_t pduLen = htons(dataLen + sizeof(uint16_t));    // length of entire pdu
	int sent = 0;           // actual amount of data sent

	// create PDU
	memcpy(sendBuf, &pduLen, sizeof(pduLen));
	memcpy(sendBuf + sizeof(pduLen), sendData, dataLen);
	
	// send PDU
	sent = send(socketNum, sendBuf, dataLen + sizeof(pduLen), 0);
	if (sent < 0) {
		perror("send call");
		exit(-1);
	}
	// printf("Amount of data sent is: %d\n", sent);
}

void recvPacket(int clientSocket, char *buf) {
	// populates provided buf with revieved packet
	uint16_t messageLen = 0;
	
	// get the header
	if (!safeRecv(clientSocket, buf, sizeof(messageLen))) {
		// connection was closed, return
		return;
	}
	
	memcpy(&messageLen, buf, sizeof(messageLen));
	messageLen = ntohs(messageLen); // convert to host network order
	
	// TODO: check for buf overflow (too long of a message)

	//now get the data from the client_socket
	if (!safeRecv(clientSocket, buf + sizeof(messageLen), messageLen - sizeof(messageLen))) {
		// connection was closed, return
		return;
	}
}

int safeRecv(int socketNum, char *buf, uint16_t len) {
	// true indicates a success, false indicates a connection closed
	uint16_t result;
	if ((result=recv(socketNum, buf, len, MSG_WAITALL)) < 0) {
		perror("recv call");
		exit(-1);
	}
	else if (result == 0) {
		// connection was closed, indicate with a 0 in message len
		// TODO: change to a flag for prog 2, make result an int
		memcpy(buf, &result, sizeof(uint16_t));
		return 0;
	}
	return 1;
}

int exitFound(char *buf, int len) {
	// checks if given string is "exit"
	if (len != 5) {
		return 0;
	}
	return !strncmp("exit\0", buf, 5);

}
