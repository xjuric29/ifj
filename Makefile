CFLAGS=-std=gnu99 -Wall -Wextra

all: parser.c symtab.c scanner.c str.c expr.c stack.c
	gcc ${CFLAGS} parser.c symtab.c scanner.c str.c expr.c stack.c -o compiler

clean:
	rm -f *.o *.out *.gch
