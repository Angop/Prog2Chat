/* Angela Kerlin
 * CPE 464-01
 * Handles the socket handle table */

#include <stdlib.h>
#include <unistd.h>

#include "khash.h" // open source hashtable library

int addSocketHandle(int socketNum, char *handle);
int updateSocketHandle(int socketNum, char *handle);
int closeSocketHandle(char *handle);
char * getSocket(char *handle);
// int getHandle(); // probably not needed
char ** getAllHandles();
