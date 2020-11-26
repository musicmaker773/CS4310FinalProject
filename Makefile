#Write the Makefile for this directory. You should make sure the dependencies are in the right order.
#Here is a link for reference: https://opensource.com/article/18/8/what-how-makefile

CC=clang
CFLAGS= -g
COMMON_OBJS = common.o

all: $(COMMON_OBJS) tcps

common.o: common.h common.c


tcps: tcpserver.c $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o tcps tcpserver.c $(COMMON_OBJS)

clean:
	rm -rf *.dSYM tcps