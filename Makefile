all: main

main: main.cpp function.cpp proxy.cpp proxy.h request.h Date.h GetRequest.h
	g++ -g  -o main main.cpp function.cpp proxy.cpp request.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
