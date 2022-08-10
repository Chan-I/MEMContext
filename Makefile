
CC=gcc -g -O0

all:test

test:test.c mcxt.o
	${CC} $^ -o $@

.phony:clean
clean:
	rm -rf *.o test 