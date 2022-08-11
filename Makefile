CC=gcc -g -O0

test:test.c mcxt.o

.phony:clean
clean:
	rm -rf *.o test 
