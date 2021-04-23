# Makefile for CPE464 tcp test code
# written by Hugh Smith - April 2019
# modified by Angela Kerlin

CC= gcc
CFLAGS= -g -Wall
LIBS = 


all:   cclient server

cclient: myClient.c networks.o gethostbyname.o sendrecv.o shared.o pollLib.o safeUtil.o 
	$(CC) $(CFLAGS) -o cclient myClient.c networks.o gethostbyname.o sendrecv.o shared.o pollLib.o safeUtil.o $(LIBS)

server: myServer.c networks.o gethostbyname.o sendrecv.o pollLib.o safeUtil.o socketHandle.o shared.o
	$(CC) $(CFLAGS) -o server myServer.c networks.o gethostbyname.o sendrecv.o pollLib.o safeUtil.o socketHandle.o shared.o $(LIBS)

testSH: pollLib.o socketHandle.o socketHandleTestcases.o safeUtil.o
	$(CC) $(CFLAGS) -o testSH socketHandleTestcases.o pollLib.o socketHandle.o safeUtil.o $(LIBS)

.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f server cclient testSH *.o




