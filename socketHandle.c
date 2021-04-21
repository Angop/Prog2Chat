/* Angela Kerlin
 * CPE 464-01
 * maps client defined handles to their socket number */

#include "socketHandle.h"

// socketHandle Global Variables
const int shHash = 33; // symbolic name for socket handle hashtable
KHASH_MAP_INIT_STR(shHash, int) // instantiate structs and methods


int addSocketHandle(int socketNum, char *handle) {
    // TODO
    // below is just a test of the hashtable itself


    return 0;
}


int updateSocketHandle(int socketNum, char *handle) {
    // TODO

    return 0;
}


int closeSocketHandle(char *handle) {
    // TODO

    return 0;
}


char * getSocket(char *handle) {
    // TODO

    return NULL;
}


char ** getAllSockets(char *handle) {
    // TODO

    return NULL;
}

