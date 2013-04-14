all: client

server: server.o
	gcc -g -o server server.o

server.o: server.c server.h
	gcc -c server.c

#ifeq ($(UNAME), Linux)
#client: client.o string.o time.o
	gcc -g -o client client.o string.o time.o -ljansson -lrt -lm
#endif

#ifeq ($(UNAME), Darwin)
client: client.o string.o time.o
	gcc -g -o client client.o string.o time.o -ljansson -lm
#endif

client.o: client.c client.h
	gcc -g -c client.c

string.o: string.c string.h
	gcc -g -c string.c

time.o: time.c time.h
	gcc -g -c time.c

clean:
	rm -f *.o server client