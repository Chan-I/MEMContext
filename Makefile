CC=gcc -std=gnu99 -Wall -Wmissing-prototypes -Wendif-labels -Wpointer-arith -Wdeclaration-after-statement -Werror=vla -Wendif-labels  \
-Wno-deprecated -Wno-deprecated-declarations -Wdisabled-optimization -fno-strict-aliasing -fwrapv -fexcess-precision=standard\

test:test.o mcxt.o

.phony:clean
clean:
	rm -rf *.o test 
