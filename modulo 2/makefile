all:
	g++ server.cpp -o server -lpthread -g
	g++ client.cpp -o client -lpthread -g
valgrind:
	g++ server.cpp -o server -lpthread -g
	g++ client.cpp -o client -lpthread -g
run-server:
	./server
run-client:
	./client
valgrind-server:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server
valgrind-client:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client 
clear:
	@rm client server