CC=gcc -Wmissing-prototypes -Wendif-labels \
-Wno-deprecated -Wno-deprecated-declarations -Wdisabled-optimization \

test:test.c mcxt.o

.phony:clean
clean:
	rm -rf *.o test 
