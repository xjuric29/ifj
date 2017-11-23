CFLAGS=-std=c99
#CFLAGS=-std=gnu99 -Wall

all: compiler

compiler: main.o scanner.o str.o parser.o ilist.o symtab.o
	gcc main.o scanner.o str.o parser.o ilist.o symtab.o -o compiler

scanner.o: scanner.c
	gcc ${CFLAGS} scanner.c -c

str.o: str.c
	gcc ${CFLAGS} str.c -c

parser.o: parser.c
	gcc ${CFLAGS} parser.c -c

ilist.o: ilist.c
	gcc ${CFLAGS} ilist.c -c

symtab.o: symtab.c
	gcc ${CFLAGS} symtab.c -c


main.o: main.c
	gcc ${CFLAGS} main.c -c

clean:
	rm -f *.o *.out *.gch
