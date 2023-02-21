all: main

main: main.cpp head.h Date.h
	g++ -g  -o main main.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
