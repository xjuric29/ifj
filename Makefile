#CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
CFLAGS=-std=gnu99 -Wall -Wextra

compiler: main.o scanner.o
	gcc main.o scanner.o -o compiler.out

scanner.o: scanner.c
	gcc ${CFLAGS} scanner.c -c

main.o: main.c scanner.h
	gcc ${CFLAGS} main.c scanner.h -c

clean:
	rm -f *.o *.out *.gch