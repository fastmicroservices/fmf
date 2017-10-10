CXXFLAGS = -std=c++14 -I../include -I../external/zstr/src -Wall
CFLAGS = -std=c11 -I../include -Wall

all: dashboard

clean:
	rm bin/*

dashboard: bin/dashboard.o ../lib/libfmf.so
	clang++ bin/dashboard.o -L../lib -lfmf -lz -o dashboard

bin/dashboard.o: dashboard.cpp ../include/config.hpp
	clang++ -c dashboard.cpp $(CXXFLAGS) -o bin/dashboard.o
