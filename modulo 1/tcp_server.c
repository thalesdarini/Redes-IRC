#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>

#define PORT 60001
//#define SERVER_IP "172.19.137.154"


int main(void)
{
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost only
    //server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_addr.s_addr = INADDR_ANY; //bind to all interfaces

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n"); 
    
    // Listen for clients:
    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");
    
    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
    
    if (client_sock < 0){
        printf("Can't accept\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    

    char sent_message[100000], recv_message[4097], parc_message[4097]; 

    //loop for continuos communication
    while (1){
        // Clean buffers:
        memset(sent_message, '\0', sizeof(sent_message));

        int tam_message;
        // Receive client's message size (number of parts the msg was divided):
        if (recv(client_sock, &tam_message, sizeof(tam_message), 0) < 0){
            printf("Couldn't receive\n");
            return -1;
        }

        // Receive each part client's message:
        int ok = 0;
        printf("Msg from client:\n");
        for (int i = 0; i < tam_message; i++){
            memset(recv_message, '\0', sizeof(recv_message));
            if (recv(client_sock, recv_message, sizeof(recv_message), 0) < 0){
                printf("Couldn't receive\n");
                return -1;
            }
            //print part of the message
            printf("%s", recv_message);

            //send confirmation to receive another part of the msg
            if (send(client_sock, &ok, sizeof(ok), 0) < 0){
                printf("Error while sending confirmation\n");
                return -1;
            }

        }
        printf("\n");

        if (strcmp("\\quit\n", recv_message) == 0) //verify if client wants to end socket
            break;

        // Respond to client:
        printf("Enter message: ");
        fgets(sent_message, 100000, stdin);
        tam_message = (strlen(sent_message)/4096) + 1;

        int exit = 0;
        if (strcmp("\\quit\n", sent_message) == 0) //verify if the server wants to end communication
            exit = 1;

        //send message size(number of parts the msg was divided)
        if (send(client_sock, &tam_message, sizeof(tam_message), 0) < 0){
            printf("Can't send\n");
            return -1;
        }

        ok=0;
        //sent each part of the msg
        for (int i = 0; i < tam_message; i++){
            memset(parc_message,'\0',sizeof(parc_message));
            strncpy(parc_message,sent_message+(i*4096), 4096);
            if(send(client_sock, parc_message, strlen(parc_message), 0) < 0){
                printf("Unable to send message\n");
                return -1;
            } 


            //confirm if another part of the msg can be sent
            if(recv(client_sock, &ok, sizeof(ok), 0) < 0){
                printf("Error while receiving server's confirmation\n");
                return -1;
            } 

        }
        printf("\n");

        if (exit) //if the server wants to end communication => close socket
            break;
    }

    

    // Closing the socket:
    close(client_sock);
    close(socket_desc);
    
    return 0;
}
