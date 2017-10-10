CXXFLAGS = -std=c++14 -I../include -I../external/zstr/src -Wall
CFLAGS = -std=c11 -I../include -Wall

all: graph

clean:
	rm graph graph.o

graph: graph.o ../lib/libfmf.so
	clang++ graph.o -L../lib -lfmf -lz -o graph

bin/graph.o: graph.cpp ../include/config.hpp
	clang++ -c graph.cpp $(CXXFLAGS) -o graph.o
