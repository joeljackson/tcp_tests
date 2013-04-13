all: server client

server: server.o
	gcc -g -o server server.o

server.o: server.c server.h
	gcc -c server.c

client: client.o string.o
	gcc -g -o client client.o string.o -ljansson -lrt -lm

client.o: client.c client.h
	gcc -g -c client.c

string.o: string.c string.h
	gcc -g -c string.c

clean:
	rm -f *.o server client