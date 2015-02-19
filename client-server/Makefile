all:  server
clean:
	rm client server
client: client.c
	g++ -o client client.c
server: server.c
	g++ -std=c++11 -o server server.c -lpthread 
