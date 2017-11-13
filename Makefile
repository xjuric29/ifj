CFLAGS=-std=gnu99 -Wall -Wextra -Werror
#CFLAGS=-std=gnu99 -Wall

compiler: main.o scanner.o str.o
	gcc main.o scanner.o str.o -o compiler.out

scanner.o: scanner.c
	gcc ${CFLAGS} scanner.c -c

str.o: str.c
	gcc ${CFLAGS} str.c -c

main.o: main.c scanner.h str.h
	gcc ${CFLAGS} main.c scanner.h str.h -c

clean:
	rm -f *.o *.out *.gch