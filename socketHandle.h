/* Angela Kerlin
 * CPE 464-01
 * Handles the socket handle table */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_HANDLE_LEN 100

int addSocketHandle(int socketNum, char *handle, int handleLen);
int closeSocketHandle(int socketNum);
int getSocket(char *handle, int handleLen);
void getAllSockets(int *socketList);
void getAllHandles(char (*handleList)[MAX_HANDLE_LEN + 1]);
int getNumEntries();
// int updateSocketHandle(int socketNum, char *handle); // probably not needed
// int getHandle(); // probably not needed