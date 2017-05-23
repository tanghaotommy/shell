OBJS = main.o exec.o parser.o
CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

nsh : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o nsh

main.o : parser.h exec.h
	$(CC) $(CFLAGS) main.c

exec.o : exec.h parser.h
	$(CC) $(CFLAGS) exec.c

parser.o : parser.h
	$(CC) $(CFLAGS) parser.c

clean:
	\rm $(OBJS) nsh

tar:
	tar cfv nsh.tar main.c parser.h parser.c exec.h exec.c
