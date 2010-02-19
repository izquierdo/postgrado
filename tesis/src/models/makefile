C++     = g++
CC	= $(C++)
NNF	= nnf
GMP	= ../../gmp-4.2
CFLAGS	= -Wall -O3 -I$(NNF) -I$(GMP) -DMP
LDFLAGS = -L$(NNF) -lnnf -L$(GMP) -lgmp

all:		models bestmodel filtermodels

models:
		$(C++) -o models models.o $(LDFLAGS)

bestmodel:
		$(C++) -o bestmodel bestmodel.o $(LDFLAGS)

filtermodels:
		$(C++) -o filtermodels filtermodels.o $(LDFLAGS)

.cc.o:
		$(C++) $(CFLAGS) -c $<

models:		models.o $(NNF)/libnnf.a
bestmodel:	bestmodel.o $(NNF)/libnnf.a
filtermodels:   filtermodels.o $(NNF)/libnnf.a
models.o:	$(NNF)/nnf.h $(NNF)/satlib.h
bestmodel.o:	$(NNF)/nnf.h $(NNF)/satlib.h
filtermodels.o:   $(NNF)/nnf.h $(NNF)/satlib.h

clean:
		rm -fr *.o *~ core models bestmodel filtermodels

