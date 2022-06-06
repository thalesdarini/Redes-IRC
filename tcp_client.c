#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 60001
#define SERVER_IP "127.0.0.1"

int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }
    
    printf("Socket created successfully\n");
    
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost only
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");
    

    /******************************/
    // ALTERAR ESSA PARTE DAS MENSAGENS PARA A ENTREGA
    char server_message[4096], client_message[4096];
    
    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Get input from the user:
    printf("Enter message: ");
    fgets(client_message, 4096, stdin);
    
    // Send the message to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    
    // Receive the server's response:
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    
    printf("Server's response: %s\n",server_message);
    /******************************/




    // Close the socket:
    close(socket_desc);
    
    return 0;
}