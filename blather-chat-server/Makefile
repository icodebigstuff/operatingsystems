CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

blather : bl_client bl_server

bl_client : simpio.o util.o bl_client.o
	$(CC) -o bl_client bl_client.o simpio.o util.o -lpthread

bl_server : bl_server.o server_funcs.o util.o
	$(CC) -o bl_server bl_server.o server_funcs.o util.o

simpio.o : simpio.c blather.h
	$(CC) -c $<

bl_client.o : bl_client.c blather.h
	$(CC) -c $<

bl_server.o : bl_server.c blather.h
	$(CC) -c $<

server_funcs.o : server_funcs.c blather.h
	$(CC) -c $<

util.o : util.c blather.h
	$(CC) -c $<


clean :
	rm -f bl_server bl_client *.o *.fifo *.log

include test_Makefile
