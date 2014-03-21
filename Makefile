

all: fast-lookup

fast-lookup: fast-lookup.o 
	g++ -ggdb3 -std=c++11 fast-lookup.o -o fast-lookup

fast-lookup.o: fast-lookup.cpp Makefile
	g++ -c -ggdb3 -std=c++11 fast-lookup.cpp


