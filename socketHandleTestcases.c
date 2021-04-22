/* Angela Kerlin
 * Contains testcases for socketHandle data structure
 */

// TODO
#include <stdlib.h>
#include <stdio.h>
#include "socketHandle.h"

int main(int argc, char *argv[]) {
    printf("------testAddOneSocket------\n");
    printf(addSocketHandle(1, "client1\0", 8) ? "pass\n" : "fail\n");

    printf("------testAddDupHandle------\n");
    printf(addSocketHandle(3, "client1\0", 8) ? "fail\n" : "pass\n");

    printf("------testRemoveSocketWhenOnlyOne------\n");
    printf(closeSocketHandle(1) ? "pass\n" : "fail\n");

    printf("------testRemoveSocketWhenNone------\n");
    printf(closeSocketHandle(3) ? "fail\n" : "pass\n");

    printf("------testAddMoreSocket------\n");
    printf(addSocketHandle(1, "client1\0", 8) ? "pass\n" : "fail\n");
    printf(addSocketHandle(2, "client2\0", 8) ? "pass\n" : "fail\n");
    printf(addSocketHandle(3, "client3\0", 8) ? "pass\n" : "fail\n");
    printf(addSocketHandle(4, "client4\0", 8) ? "pass\n" : "fail\n");
    printf(addSocketHandle(5, "client5\0", 8) ? "pass\n" : "fail\n");

    printf("------testRemoveSocketWhenMany------\n");
    printf(closeSocketHandle(3) ? "pass\n" : "fail\n");

    printf("------getSockets------\n");
    printf(getNumEntries() == 4 ? "pass\n" : "fail\n");
    int entries = getNumEntries();
    int socketList[entries];
    getAllSockets(socketList);
    int i = 0;
    for (i=0;i < entries;i++) {
        printf("%d, ", socketList[i]);
    }
    printf("\n");

    printf("------getHandles------\n");
    char handleList[entries][101];
    getAllHandles(handleList);
    i = 0;
    for (i=0;i < entries;i++) {
        printf("%s, ", handleList[i]);
    }
    printf("\n");

    printf("------getSocketExists------\n");
    printf(getSocket("client2\0", 8) == 2 ? "pass\n" : "fail\n");

    printf("------getSocketDoesntExist------\n");
    printf(getSocket("client3\0", 8) == -1 ? "pass\n" : "fail\n");
}