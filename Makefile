CXXFLAGS = -std=c++14 -Iinclude -Wall

example1: example1.o environment.o
	clang++ example1.o environment.o -o example1

example1.o: example1.cpp include/config.hpp
	clang++ -c example1.cpp $(CXXFLAGS)

environment.o: impl/environment.cpp include/config.hpp
	clang++ -c impl/environment.cpp $(CXXFLAGS)
