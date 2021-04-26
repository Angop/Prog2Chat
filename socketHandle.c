/* Angela Kerlin
 * CPE 464-01
 * maps client defined handles to their socket number
 * as a linked list structure */

#include "socketHandle.h"
#include "pollLib.h"


typedef struct ShNode *shNode;
struct ShNode {
    int sNum;
    char handle[MAX_HANDLE_LEN + 1];
    int handleLen;
    shNode next;
};

void shClose(shNode sh);
// TODO: WRITE SAFE MALLOC AND CALLOC FUNCTIONS and replace all the uses

// socketHandle Global Variables
// dd is it better to make it global in this file or pass around the root?
static shNode root = NULL; // initialize the list to null/empty
static int shLen = 0;


int addSocketHandle(int socketNum, char *handle, int handleLen) {
    // Adds a new socket handle to the list unless its a duplicate handle
    // returns 1 on success, 0 on duplicate handle

    // Create the new node structure dd make into helper?
    shNode new = (shNode)malloc(sizeof(struct ShNode));
    new->sNum = socketNum;
    memcpy(new->handle, handle, handleLen * sizeof(int8_t));
    new->handleLen = handleLen;
    new->next = NULL;
    
    // place the node in the linked list, unless it encounters an identical handle
    shNode cur = root;
    shNode last = NULL;
    while (cur != NULL) {
        if (cur->handleLen == handleLen) {
            // compare the two handles
            if (!strncmp(handle, cur->handle, handleLen)) {
                // identical handle exists!
                free(new);
                return 0;
            }
        }
        last = cur;
        cur = cur->next;
    }

    // place at the end of list
    if (last == NULL) {
        root = new;
    }
    else {
        last->next = new;
    }

    shLen++;
    return 1;
}


int closeSocketHandle(int socketNum) {
    // closes the socket, returns 1 on sucess and 0 if it does not exist
    shNode cur = root;
    shNode last = NULL;
    shNode temp = NULL;

    while (cur != NULL) {
        if (cur->sNum == socketNum) {
            // found the socket! close it
            temp = cur->next;
            shClose(cur);
            // remove node from list
            if (last) { last->next = temp; } else{ root = temp; }
            return 1;
        }
        last = cur;
        cur = cur->next;
    }
    // handle did not exist
    return 0;
}


void shClose(shNode sh) {
    // close socket, remove it from pollset, free node
    removeFromPollSet(sh->sNum); // TODO UNCOMMENT THESE
    close(sh->sNum);
    free(sh);
    shLen--;
}


int getSocket(char *handle, int handleLen) {
    // gets the socket associated with the given handle, if it does not exist then returns -1
    shNode cur = root;
    while (cur != NULL) {
        if (cur->handleLen == handleLen) {
            // compare the two handles
            if (!strncmp(handle, cur->handle, handleLen)) {
                // handle exists
                return cur->sNum;
            }
        }
        cur = cur->next;
    }
    // handle does not exist
    return -1;
}


void getAllSockets(int *socketList) {
    // fills provided list with socket numbers
    // assumes the list is the correct size dd bad idea
    shNode cur = root;
    int i = 0;
    while (cur != NULL) {
        socketList[i] = cur->sNum;
        cur = cur->next;
        i++;
    }
}


void getAllHandles(char (*handleList)[101]) {
    // fills provided list with handles
    // assumes the list is the correct size dd bad idea
    shNode cur = root;
    int i = 0;
    while (cur != NULL) {
        memcpy(handleList[i], cur->handle, cur->handleLen);
        cur = cur->next;
        i++;
    }
}


int getNumEntries() {
    // returns the number of entries in the linked list
    return shLen;
}


void printAllEntries() {
    shNode cur = root;
    int i = 0;
    printf("ENTRIES:\n");
    while (cur != NULL) {
        printf("\tHandle: %s Socket %d,\n", cur->handle, cur->sNum);
        cur = cur->next;
        i++;
    }
}