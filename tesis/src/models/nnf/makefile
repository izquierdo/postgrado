OBJS	= nnf.o satlib.o
GMP     = ../../../gmp-4.2

INCLUDE = -I$(GMP)
CFLAGS	= -O3 -Wall -DMP
CC	= gcc
C++     = g++

libnnf.a:	$(OBJS)
		ar rv libnnf.a $(OBJS)

.cc.o:
		$(C++) $(CFLAGS) -c $< $(INCLUDE)

clean:
		rm -f libnnf.a $(OBJS) *~ core

# DO NOT DELETE

nnf.o: nnf.h satlib.h utils.h
satlib.o: nnf.h satlib.h
