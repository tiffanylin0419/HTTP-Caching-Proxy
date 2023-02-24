all: main

main: main.cpp function.cpp proxy.cpp proxy.h request.h Date.h response.h
	g++ -g  -o main main.cpp function.cpp proxy.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
