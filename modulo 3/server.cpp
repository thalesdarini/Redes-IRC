#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#define MAX_LEN 60
#define NUM_COLORS 6

using namespace std;

struct terminal
{
	int id;
	string name;
	string channel;
	string ip;
	bool mute;
	bool admin;
	int socket;
	thread th;
};

int server_socket;
vector<terminal> clients;
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m","\033[36m"};
int seed=0;
bool mut=false, adm=false;
mutex cout_mtx,clients_mtx;

struct sockaddr_in client;
void catch_ctrl_c(int signal);
string color(int code);
void set_name(int id, char name[]);
void set_channel(int id, char channel[]);
void shared_print(string str, bool endLine = true);
int broadcast_message(string message, int sender_id, string channel);
int send_message(string message, int sender_id);
int broadcast_message(int num, int sender_id, string channel);
void end_connection(int id);
void kick(string name, string channel);
void mute(string name, string channel);
void unmute(string name, string channel);
void whois(string name, int id, string channel);
void handle_client(int client_socket, int id);

int main()
{
	if((server_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(10000);
	server.sin_addr.s_addr=INADDR_ANY;
	bzero(&server.sin_zero,0);

	if((bind(server_socket,(struct sockaddr *)&server,sizeof(struct sockaddr_in)))==-1)
	{
		perror("bind error: ");
		exit(-1);
	}

	if((listen(server_socket,8))==-1)
	{
		perror("listen error: ");
		exit(-1);
	}

	int client_socket;
	unsigned int len=sizeof(sockaddr_in);

	signal(SIGINT, catch_ctrl_c);
	cout<<colors[NUM_COLORS-1]<<"\n\t  ====== Welcome to the chat-room ======   "<<endl<<def_col;

	while(1)
	{
		if((client_socket=accept(server_socket,(struct sockaddr *)&client,&len))==-1)
		{
			//perror("accept error: ");
			cout << "Stopped accepting new connections" << endl;
			break;
		}
		seed++;
		thread t(handle_client,client_socket,seed);

		lock_guard<mutex> guard(clients_mtx);
		clients.push_back(
			{
				seed, 
				string("Anonymous"),
				string("Anyone"),
				string("ip"),
				mut,
				adm,
				client_socket,
				(move(t))
			}
		);
	}

	// Joins threads
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		if (clients[i].th.joinable())
			clients[i].th.join();
	}

	if (server_socket != -1) 
		close(server_socket);

	return 0;
}


// Handler for "Ctrl + C"
void catch_ctrl_c(int signal) 
{
	cout << "\33[2K\rClosing server and terminating program..." << endl;
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		shutdown(clients[i].socket, SHUT_RDWR);
		close(clients[i].socket);
	}

	close(server_socket);
	server_socket = -1;
}

string color(int code)
{
	return colors[code%NUM_COLORS];
}

// Set name of client
void set_name(int id, char name[])
{	
	char str[50];
	inet_ntop(AF_INET,&client.sin_addr.s_addr, str, sizeof(str));
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
			if(clients[i].id==id)	
			{
				clients[i].name=string(name);
				clients[i].mute = false;
				clients[i].ip = string(str);
				
			}
	}	
}

void set_channel(int id, char channel[])
{
	lock_guard<mutex> guard(clients_mtx);
	bool admin = true; 
	int novo;
	string Channel = string(channel);
	
	for(int i=0; i<clients.size(); i++)
	{
			if(clients[i].id==id)	
			{
				clients[i].channel=Channel;
				novo = i;
			}

			if(clients[i].id!=id && Channel.compare(clients[i].channel)==0)
			{
				admin = false;
			}
	}

	clients[novo].admin = admin;	
}

// For synchronisation of cout statements
void shared_print(string str, bool endLine)
{	
	lock_guard<mutex> guard(cout_mtx);
	cout<<str;
	if(endLine)
		cout<<endl;
}

// Broadcast message to all clients except the sender
int broadcast_message(string message, int sender_id, string channel)
{
	lock_guard<mutex> guard(clients_mtx);
	char temp[MAX_LEN];
	memset(temp, '\0', sizeof(temp));
	strcpy(temp, message.c_str());
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id!=sender_id && channel.compare(clients[i].channel)==0)
		{
			//send(clients[i].socket, message.c_str(), message.length() + 1, 0);
			send(clients[i].socket, temp, sizeof(temp), 0);
		}
	}	

	return 0;	
}

// Send message to a single client
int send_message(string message, int sender_id)
{
	lock_guard<mutex> guard(clients_mtx);
	char temp[MAX_LEN];
	memset(temp, '\0', sizeof(temp));
	strcpy(temp, message.c_str());
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id==sender_id)
		{
			//send(clients[i].socket,message.c_str(),message.length() + 1, 0);
			send(clients[i].socket,temp,sizeof(temp), 0);
		}
	}	

	return 0;	
}

// Broadcast a number to all clients except the sender
int broadcast_message(int num, int sender_id, string channel)
{
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id!=sender_id && channel.compare(clients[i].channel)==0)
		{
			send(clients[i].socket,&num,sizeof(num),0);
		}
	}	

	return 0;	
}

void end_connection(int id)
{
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id==id)
		{
			clients[i].th.detach();
			close(clients[i].socket);
			clients.erase(clients.begin()+i);
			break;
		}
	}				
}

void kick(string name, string channel)
{
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		if(channel.compare(clients[i].channel)==0 && clients[i].name.compare(name)==0)
		{
			clients[i].th.detach();
			close(clients[i].socket);
			clients.erase(clients.begin()+i);
			break;
		}
	}				
}

void mute(string name, string channel)
{
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		if(channel.compare(clients[i].channel)==0 && clients[i].name.compare(name)==0)
		{
			clients[i].mute = true;
			break;
		}
	}	

}

void unmute(string name, string channel)
{
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		if(channel.compare(clients[i].channel)==0 && clients[i].name.compare(name)==0)
		{
			clients[i].mute = false;
			break;
		}
	}	

}

void whois(string name, int id, string channel)
{
	lock_guard<mutex> guard(clients_mtx);
	for(int i=0; i<clients.size(); i++)
	{
		if(channel.compare(clients[i].channel)==0 && clients[i].name.compare(name)==0)
		{
			send_message(string("#IP"), id);
			break;
		}
	}	
}

void handle_client(int client_socket, int id)
{
	char name[MAX_LEN],str[MAX_LEN], channel[MAX_LEN];
	recv(client_socket,name,sizeof(name),0);
	set_name(id,name);	
	recv(client_socket,channel,sizeof(channel),0);
	set_channel(id,channel);


	// Display welcome message
	string welcome_message=string(name)+string(" has joined com id ") + to_string(id);
	broadcast_message(string("#NULL"),id, string(channel));	
	broadcast_message(id,id, string(channel));								
	broadcast_message(welcome_message,id, string(channel));	
	shared_print(color(id)+welcome_message+def_col);
	
	while(1)
	{
		int tam_message;
		int bytes_received=recv(client_socket,&tam_message,sizeof(tam_message),0);
		if(bytes_received<=0){
			return;
		}
		bytes_received=recv(client_socket,str,sizeof(str),0);
		string comp = string(str);
		if(bytes_received<=0){
			return;
		}
		if(strcmp(str,"/quit")==0)
		{
			// Display leaving message
			string message=string(name)+string(" has left");		
			broadcast_message(string("#NULL"),id, string(channel));			
			broadcast_message(id,id, string(channel));						
			broadcast_message(message,id, string(channel));
			shared_print(color(id)+message+def_col);
			end_connection(id);							
			return;
		}
		if(clients[id-1].admin == true){
			if(comp.length() >=6 && comp.substr(0,5).compare("/kick")==0 ){
				kick(comp.substr(6), string(channel));
				continue;
			}
			else if(comp.length() >=6 && comp.substr(0,5).compare("/mute")==0 ){
				mute(comp.substr(6), string(channel));
				continue;
			}
			else if(comp.length() >=8 && comp.substr(0,7).compare("/unmute")==0){
				unmute(comp.substr(8), string(channel));
				continue;
			}
			else if(comp.length() >=7 && comp.substr(0,6).compare("/whois")==0){
				whois(comp.substr(7), id, string(channel));
				continue;
			}
		}
		if (strcmp(str, "/ping") == 0)
		{
			send_message(string("#PONG"), id);
		}
		else {
			if(clients[id-1].mute == false){
				broadcast_message(string(name),id,string(channel));					
				broadcast_message(id,id, string(channel));	
				broadcast_message(tam_message,id, string(channel));

				shared_print(color(id)+name+" : "+def_col+str, (tam_message == 1));	
				broadcast_message(string(str),id, string(channel));

				while (--tam_message > 0) {
					bytes_received=recv(client_socket,str,sizeof(str),0);
					if(bytes_received<=0){
						return;
					}
					shared_print(str, (tam_message == 1));	
					broadcast_message(string(str),id, string(channel));
				}
			}
		}
	}	
}