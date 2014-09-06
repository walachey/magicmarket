# Makefile -- makefile for lightweight backpropagation neural network++
LIB_SOURCES := network.cc trainer.cc iomanage.cc iomanagelwnnfann.cc iomanagebinary.cc shuffle.cc
LIB_OBJECTS := $(LIB_SOURCES:.cc=.o)
EXE_SOURCES := generate_sigma.c
EXE_OBJECTS := $(EXE_SOURCES:.c=.o)
EXECUTABLES := $(EXE_SOURCES:.c=)
CFLAGS:= -O3 -finline-functions -fPIC -DLWNNPLUS_VERSION=$(VERSION) -Wall -Winline -pedantic
CC := g++
# change this value to change the size of interpolation table
N_TABLE_ENTRIES:=2000

all : sigmoidal.cc liblwnnplus.so

%.o: %.cc
		$(CC) $(CFLAGS) -c $< -o $@ 

liblwnnplus.so: $(LIB_OBJECTS)
	$(CC) $(LIB_OBJECTS) -shared -o liblwnnplus.so


sigmoidal.cc : generate_sigma
	./generate_sigma -n $(N_TABLE_ENTRIES) > sigmoidal.cc


generate_sigma.o:
	gcc -g -O2 -Wall -W -Winline -c -o generate_sigma.o generate_sigma.c

generate_sigma : generate_sigma.o
	gcc -o $@ $^ -lm

clean-dist-part :
	-rm -f sigmoidal.cc
	-rm -f $(LIB_OBJECTS) $(EXE_OBJECTS) $(EXECUTABLES)

clean : clean-dist-part clean-backup clean-lib

clean-backup :
	-rm -f  *~

clean-lib:
	rm -f *.so








