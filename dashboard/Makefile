CXXFLAGS = -std=c++14 -I../include -Wall
CFLAGS = -std=c11 -I../include -Wall

all: dashboard

clean:
	rm bin/*

dashboard: bin/dashboard.o bin/environment.o bin/inmem.o bin/multiple.o bin/mongoose.o bin/http.o bin/eureka.o
	clang++ bin/dashboard.o bin/environment.o bin/inmem.o bin/multiple.o bin/http.o bin/mongoose.o bin/eureka.o -o dashboard

bin/dashboard.o: dashboard.cpp ../include/config.hpp
	clang++ -c dashboard.cpp $(CXXFLAGS) -o bin/dashboard.o

bin/environment.o: ../impl/environment.cpp ../include/config.hpp
	clang++ -c ../impl/environment.cpp $(CXXFLAGS) -o bin/environment.o

bin/inmem.o: ../impl/inmem.cpp ../include/config.hpp
	clang++ -c ../impl/inmem.cpp $(CXXFLAGS) -o bin/inmem.o

bin/multiple.o: ../impl/multiple.cpp ../include/config.hpp
	clang++ -c ../impl/multiple.cpp $(CXXFLAGS) -o bin/multiple.o

bin/http.o: ../impl/http.cpp ../include/config.hpp ../include/endpoint.hpp ../include/mongoose.h
	clang++ -c ../impl/http.cpp $(CXXFLAGS) -o bin/http.o

bin/mongoose.o: ../mongoose/mongoose.c
	clang -c ../mongoose/mongoose.c $(CFLAGS) -o bin/mongoose.o

bin/eureka.o: ../impl/eureka.cpp ../include/discovery.hpp
	clang++ -c ../impl/eureka.cpp $(CXXFLAGS) -o bin/eureka.o
