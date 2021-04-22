/* Angela Kerlin
 * Handles sending and recieving for both the client and server
 */

#include "sendrecv.h"
int safeRecv(int socketNum, char *buf, uint16_t len);
void fillChatHeader(uint16_t pduLen, uint8_t flag, char *buf);

void sendPacket(int socketNum, char *sendData, int dataLen, uint8_t flag) {
	// TODO: check for buf overflow (too long of a message)
	char sendBuf[MAXBUF];   // data buffer
	int sent = 0;           // actual amount of data sent
	
	fillChatHeader(dataLen + HEADER_BYTES, flag, sendBuf);
	if (sendData) {
		memcpy(sendBuf + HEADER_BYTES, sendData, dataLen); // insert message into buf
	}
	
	// send PDU
	sent = send(socketNum, sendBuf, dataLen + HEADER_BYTES, 0);
	if (sent < 0) {
		perror("send call");
		exit(-1);
	}
	// printf("Amount of data sent is: %d\n", sent);
}

void fillChatHeader(uint16_t pduLen, uint8_t flag, char *buf) {
    // fills the first 3 bytes of given buffer with chat header
	pduLen = htons(pduLen);    // length of entire pdu
	memcpy(buf, &pduLen, sizeof(uint16_t));
	memcpy(buf + sizeof(uint16_t), &flag, sizeof(uint8_t));
}

uint8_t recvPacket(int clientSocket, char *buf) {
	// populates provided buf with revieved packet, returns flag or -1 if connection closed
	uint16_t messageLen = 0;
	
	// get the header
	if (!safeRecv(clientSocket, buf, sizeof(messageLen))) {
		return -1;
	}
	
	memcpy(&messageLen, buf, sizeof(messageLen));
	messageLen = ntohs(messageLen); // convert to host network order
	
	// TODO: check for buf overflow (too long of a message)

	//now get the data from the client_socket
	if (!safeRecv(clientSocket, buf + sizeof(messageLen), messageLen - sizeof(messageLen))) {
		return -1;
	}
	return buf[HEADER_BYTES]; // HEADER_BYTES is the location of the flag
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
