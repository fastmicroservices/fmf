CXXFLAGS = -std=c++14 -Iinclude -Iexternal/zstr/src -Wall -fPIC
CFLAGS = -std=c11 -Iinclude -Wall -fPIC

all: example1 dashboard/dashboard graph/graph

dashboard/dashboard: dashboard/dashboard.cpp
	pushd dashboard && make && popd

graph/graph: graph/graph.cpp
	pushd graph && make && popd

clean:
	rm *.o example1 lib/libfmf.so
	pushd dashboard && make clean && popd
	pushd graph && make clean && popd

example1: example1.o lib/libfmf.so
	clang++ example1.o -L./lib -lfmf -o example1

lib/libfmf.so: environment.o inmem.o multiple.o mongoose.o http.o eureka.o registeringendpoint.o
	clang++ -fPIC -shared environment.o inmem.o multiple.o http.o mongoose.o eureka.o registeringendpoint.o -lz -o lib/libfmf.so

example1.o: example1.cpp include/config.hpp
	clang++ -c example1.cpp $(CXXFLAGS)

environment.o: impl/environment.cpp include/config.hpp
	clang++ -c impl/environment.cpp $(CXXFLAGS)

inmem.o: impl/inmem.cpp include/config.hpp
	clang++ -c impl/inmem.cpp $(CXXFLAGS)

multiple.o: impl/multiple.cpp include/config.hpp
	clang++ -c impl/multiple.cpp $(CXXFLAGS)

http.o: impl/http.cpp include/config.hpp include/endpoint.hpp include/mongoose.h
	clang++ -c impl/http.cpp $(CXXFLAGS)

mongoose.o: mongoose/mongoose.c
	clang -c mongoose/mongoose.c $(CFLAGS)

eureka.o: impl/eureka.cpp include/discovery.hpp
	clang++ -c impl/eureka.cpp $(CXXFLAGS)

registeringendpoint.o: impl/registeringendpoint.cpp
	clang++ -c impl/registeringendpoint.cpp $(CXXFLAGS)
