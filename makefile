all: server

server:
	gcc -o server *.c -Wall -pthread

clean: server
	rm server