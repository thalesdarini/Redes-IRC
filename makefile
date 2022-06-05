all:
	gcc tcp_server.c -o tcp_server
	gcc tcp_client.c -o tcp_client
run-server:
	./tcp_server
run-client:
	./tcp_client
clear:
	@rm tcp_server tcp_client