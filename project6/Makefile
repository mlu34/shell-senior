CC = gcc
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors \
	-fstack-protector-all -Wextra
 
PROGS = d8sh 

all: $(PROGS)

d8sh: d8sh.o lexer.o parser.tab.o executor.o
	$(CC) -lreadline -o d8sh d8sh.o lexer.o parser.tab.o executor.o

lexer.o: lexer.c lexer.h 
	$(CC) $(CFLAGS) -c lexer.c 

parser.tab.o: parser.tab.c command.h
	$(CC) $(CFLAGS) -c parser.tab.c

executor.o: executor.c command.h
	$(CC) $(CFLAGS) -c executor.c
 
d8sh.o: d8sh.c lexer.h executor.h
	$(CC) $(CFLAGS) -c d8sh.c

clean:
	@echo "Removing object, executable, and core files."
	rm -f *.o *~core core.*
