CC=g++
CFLAGS=-std=c++11 -pedantic
LIBS=-lm

all: bms


bms: bms.cpp
	$(CC) $(CFLAGS) $^ -o $@ 

clean:
	rm -f *.o bms 
