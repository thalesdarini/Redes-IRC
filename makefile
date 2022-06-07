all:
	gcc tcp_server.c -o tcp_server
	gcc tcp_client.c -o tcp_client
	gcc tcp_server2.c -o tcp_server2
	gcc tcp_client2.c -o tcp_client2
run-server:
	./tcp_server
run-server2:
	./tcp_server2
run-client:
	./tcp_client
run-client2:
	./tcp_client2
clear:
	@rm tcp_server tcp_client tcp_client2 tcp_server2