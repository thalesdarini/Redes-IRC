all:
	gcc tcp_server.c -o tcp_server
	gcc tcp_client.c -o tcp_client
run-server:
	stty -icanon
	./tcp_server
run-client:
	stty -icanon
	./tcp_client
clear:
	@rm tcp_client tcp_server