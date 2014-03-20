

all: fast-lookup

fast-lookup: fast-lookup.o 
	g++ f-std=c++11 fast-lookup.o -o fast-lookup

fast-lookup.o: fast-lookup.cpp Makefile
	g++ -c -g -std=c++11 fast-lookup.cpp


