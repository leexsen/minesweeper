CFLAGG = -Ofast -Wall -pthread -std=gnu99

all: server client
server: authentication.o error.o server.o server_network.o socket_queue.o thread_pool.o game.o leaderboard.o
	gcc $(CFLAGG) -o server $^

client: client.o client_network.o error.o
	gcc $(CFLAGG) -o client $^

authentication.o: authentication.c
	gcc $(CFLAGG) -c $^

error.o: error.c
	gcc $(CFLAGG) -c $^

server.o: server.c 
	gcc $(CFLAGG) -c $^

server_network.o: server_network.c
	gcc $(CFLAGG) -c $^

client.o: client.c 
	gcc $(CFLAGG) -c $^

client_network.o: client_network.c
	gcc $(CFLAGG) -c $^

socket_queue.o: socket_queue.c
	gcc $(CFLAGG) -c $^

thread_pool.o: thread_pool.c
	gcc $(CFLAGG) -c $^

game.o: game.c
	gcc $(CFLAGG) -c $^

leaderboard.o: leaderboard.c
	gcc $(CFLAGG) -c $^

clean: *.o 
	rm $^ 
	rm client server
