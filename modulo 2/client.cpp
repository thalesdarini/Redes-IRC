#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#define MAX_LEN 4096
#define NUM_COLORS 6

using namespace std;

bool exit_flag=false;
thread t_send, t_recv;
int client_socket;
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};

void catch_ctrl_c(int signal);
string color(int code);
int eraseTerminalLine();
void send_message(int client_socket);
void recv_message(int client_socket);

int main()
{	
	string command;
	while(1){

		signal(SIGINT, SIG_IGN);
		cout << endl << "Digite um dos comandos abaixo:" << endl;
		cout << "/connect	conecta ao servidor" << endl;
		cout << "/exit		fecha a aplicação" << endl << endl;
		cin >> command;
		cin.clear();

		if(command == "/connect"){

			if((client_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
			{
				perror("socket: ");
				exit(-1);
			}
			
			struct sockaddr_in client;
			client.sin_family=AF_INET;
			client.sin_port=htons(10000); // Port no. of server
			client.sin_addr.s_addr=INADDR_ANY;
			//client.sin_addr.s_addr=inet_addr("127.0.0.1"); // Provide IP address of server
			//bzero(&client.sin_zero,0);

			if((connect(client_socket,(struct sockaddr *)&client,sizeof(struct sockaddr_in)))==-1)
			{
				perror("connect: ");
				exit(-1);
			}
			
			cout<<"Enter your name : ";
			char name[MAX_LEN];
			memset(name, '\0', sizeof(name));
			cin >> name;

			send(client_socket,name,sizeof(name),0);

			signal(SIGINT, catch_ctrl_c);
			cout<<colors[NUM_COLORS-1]<<"\n\t  ====== Welcome to the chat-room ======   "<<endl<<def_col;

			exit_flag = false;
			thread t1(send_message, client_socket);
			thread t2(recv_message, client_socket);

			t_send=move(t1);
			t_recv=move(t2);

			if(t_send.joinable()){
				t_send.join();
			}
			if(t_recv.joinable()){
				t_recv.join();
			}
			
		} 
		else if (command == "/exit"){
			break;
		}
		else{
			cout << "Comando inválido" << endl;
		}
		
	}
			
	return 0;
}

// Handler for "Ctrl + C"
void catch_ctrl_c(int signal) 
{
	eraseTerminalLine();
	cout << "To end connection, type '/quit'" << endl;
	cout<<colors[1]<<"You : "<<def_col;
	fflush(stdout);
}

string color(int code)
{
	return colors[code%NUM_COLORS];
}

// Erase last line from terminal
int eraseTerminalLine()
{
	cout << "\33[2K\r";

	return 0;
}

void end_connection(int client_socket) {
	if (exit_flag == false){
		exit_flag=true;
		shutdown(client_socket, SHUT_RDWR);

		// wait for connection to be properly closed before closing socket
		bool try_connection = true;
		while (try_connection) {
			char test[10];
			int i = recv(client_socket,test,sizeof(test),0);
			if (i <= 0) try_connection = false;
		}
		close(client_socket);
	}
}

// Send message to everyone
void send_message(int client_socket)
{
	while(1)
	{
		cout<<colors[1]<<"You : "<<def_col;

		// read full message
		string full_message;
		cin >> ws; // ignore whitespaces
		getline(cin, full_message);

		// in case this flag is set, the function returns after you type something
		if(exit_flag) 
			return;
		
		// get how much the message has to be divided to fit the char limit
		int max_len_minus = MAX_LEN-1;
		int tam_message = (full_message.length()%max_len_minus == 0) 
			? full_message.length()/max_len_minus
			: full_message.length()/max_len_minus + 1;

		// send message count
		int bytes_sent;
		bytes_sent = send(client_socket, &tam_message , sizeof(tam_message), 0);
		if (bytes_sent == -1) {
			cout << "\33[2K\rConnection to server was lost" << endl;
			end_connection(client_socket);
			return;
		}

		
		// send each message
		char str[MAX_LEN];
		memset(str, '\0', sizeof(str));
		for (int i = 0; i < tam_message; i++){
			strncpy(str, full_message.c_str()+(i*max_len_minus), max_len_minus);
			
			int bytes_sent = send(client_socket,str,sizeof(str), MSG_NOSIGNAL);
			if(bytes_sent == -1 || strcmp(str,"/quit")==0){
				cout << "\33[2K\rConnection to server was closed" << endl;
				end_connection(client_socket);
				return;
			}
		}
	}		
}

// Receive message
void recv_message(int client_socket)
{
	while(1)
	{
		if(exit_flag)
			return;
		char name[MAX_LEN], str[MAX_LEN];
		int color_code;
		int bytes_received=recv(client_socket,name,sizeof(name),0);
		if(bytes_received<=0){
			cout << "\33[2K\rConnection to server was closed" << endl;
			cout << "Type something to exit" << endl;
			end_connection(client_socket);
			return;
		}
		eraseTerminalLine();
		if(strcmp(name,"#PONG")==0){
			cout<<"pong!"<<endl;
		}
		else if(strcmp(name,"#NULL")==0) {
			recv(client_socket,&color_code,sizeof(color_code),0);
			recv(client_socket,str,sizeof(str),0);
			cout<<color(color_code)<<str<<endl;
		}
		else{ // it is a message
			recv(client_socket,&color_code,sizeof(color_code),0);
			int tam_message;
			recv(client_socket,&tam_message,sizeof(tam_message),0);

			cout<<color(color_code)<<name<<" : ";
			for (int i = 0; i < tam_message; i++) {
				recv(client_socket,str,sizeof(str),0);
				cout<<def_col<<str;
			}
			cout << endl;
		}
		cout<<colors[1]<<"You : "<<def_col;
		fflush(stdout);
	}	
}