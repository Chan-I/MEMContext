.PHONY:clean

TARGET := test

OBJS := test.o \
		mcxt.o

CC := gcc
CFLAGS := -std=gnu99 -O2\
		  -Wall -Wmissing-prototypes \
		  -Wendif-labels -Wpointer-arith \
		  -Wdeclaration-after-statement 
		  -Werror=vla -Wendif-labels  \
		  -Wno-deprecated -Wno-deprecated-declarations \
		  -Wdisabled-optimization -fno-strict-aliasing \
		  -fwrapv -fexcess-precision=standard


$(TARGET):$(OBJS)


clean:
	rm -rf *.o test 
