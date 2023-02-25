all: main

main: main.cpp request.h head.h
	g++ -g  -o main main.cpp request.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
