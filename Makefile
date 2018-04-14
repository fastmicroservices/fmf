DEBUG_FLAGS = -DCS_ENABLE_DEBUG
CXXFLAGS = -std=c++14 -Iinclude -Iexternal/zstr/src -Wall -fPIC -DMG_ENABLE_SSL
CFLAGS = -std=c11 -Iinclude -Wall -fPIC -DMG_ENABLE_SSL -I/usr/local/opt/openssl/include
ALL_FMF_OBJECTS = environment.o inmem.o multiple.o mongoose.o http.o eureka.o registeringendpoint.o curl.o
FMF_LD = -lz -lssl -lcrypto -lcurl

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

curl.o: impl/curl.cpp include/config.hpp include/endpoint.hpp
	clang++ -c impl/curl.cpp $(CXXFLAGS)

lib/libfmf.so: $(ALL_FMF_OBJECTS)
	mkdir -p out
	mkdir -p lib
	printf "#include \"include/config.hpp\"\nchar const *FMF::Version::__fmf_commit_slug = \"`git show-ref | head -n1 | cut -d" " -f1`\";\n" | clang++ -xc++ - -c -o out/ver.o $(CXXFLAGS)
	clang++ -fPIC -shared $(ALL_FMF_OBJECTS) out/ver.o $(FMF_LD) -o lib/libfmf.so -L/usr/local/opt/openssl/lib

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

mongoose/mongoose.c:
	curl https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c > mongoose/mongoose.c

include/mongoose.h:
	curl https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h > include/mongoose.h

mongoose.o: mongoose/mongoose.c include/mongoose.h
	clang -c mongoose/mongoose.c $(CFLAGS)

eureka.o: impl/eureka.cpp include/discovery.hpp
	clang++ -c impl/eureka.cpp $(CXXFLAGS)

registeringendpoint.o: impl/registeringendpoint.cpp
	clang++ -c impl/registeringendpoint.cpp $(CXXFLAGS)
