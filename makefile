all:
	gcc tcp_server.c -o tcp_server
	gcc tcp_client.c -o tcp_client
	gcc server.c -o server
	gcc client.c -o client
run-server:
	./tcp_server
run-client:
	./tcp_client
run-client1:
	./client
run-server1:
	./server
clear:
	@rm tcp_server tcp_client