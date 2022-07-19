all:
	g++ server.cpp -o server -lpthread
	g++ client.cpp -o client -lpthread
run-server:
	stty -icanon
	./server
run-client:
	stty -icanon
	./client
clear:
	@rm client server