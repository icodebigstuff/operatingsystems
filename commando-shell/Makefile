# See Makefile-commented for explanation
CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

commando : cmd.o cmdcol.o commando.o util.o
	$(CC) -o commando cmdcol.o cmd.o commando.o util.o
	@echo commando is ready

commando.o : commando.c util.c commando.h
	$(CC) -c commando.c

cmdcol.o : cmdcol.c commando.h
	$(CC) -c $<

cmd.o : cmd.c commando.h
	$(CC) -c $<


test : commando

	./testy test_commando.org $(testnum)

clean:
	rm -f commando test_commando *.o

include test_Makefile

