CXXFLAGS = -std=c++14 -Iinclude -Wall

all: example1

clean:
	rm *.o example1

example1: example1.o environment.o inmem.o multiple.o
	clang++ example1.o environment.o inmem.o multiple.o -o example1

example1.o: example1.cpp include/config.hpp
	clang++ -c example1.cpp $(CXXFLAGS)

environment.o: impl/environment.cpp include/config.hpp
	clang++ -c impl/environment.cpp $(CXXFLAGS)

inmem.o: impl/inmem.cpp include/config.hpp
	clang++ -c impl/inmem.cpp $(CXXFLAGS)

multiple.o: impl/multiple.cpp include/config.hpp
	clang++ -c impl/multiple.cpp $(CXXFLAGS)
