all: server

server: server.o
	gcc -g -o server server.o

server.o: server.c server.h
	gcc -c server.c

clean:
	rm -f server.o server