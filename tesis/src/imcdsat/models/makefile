C++     = g++
CC	= $(C++)
NNF	= ./nnf
CFLAGS	= -Wall -O3 -I$(NNF) #-DMP
LDFLAGS = -L$(NNF) -lnnf #-lgmp

models:		models.o $(NNF)/libnnf.a
		$(C++) -o models models.o $(CFLAGS) $(LDFLAGS)

models.o:	models.cc $(NNF)/nnf.h $(NNF)/satlib.h
		$(C++) $(CFLAGS) -c models.cc

clean:
		rm -fr *.o *~ core models

