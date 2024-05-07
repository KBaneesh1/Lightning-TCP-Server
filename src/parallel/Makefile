client: ./client/client.cpp
	g++ ./client/client.cpp -o ./client/client `pkg-config --cflags --libs gtkmm-4.0`
	./client/client

server: ./server/main.cpp
	g++ ./server/main.cpp -o ./server/server 


run_server: server
	./server/server 8000

run_client: client
	./client/client 