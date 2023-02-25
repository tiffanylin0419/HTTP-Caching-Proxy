all: main

main: main.cpp connect_func.cpp proxy.cpp proxy.h request.h Date.h response.h
	g++ -g  -o main main.cpp connect_func.cpp proxy.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
