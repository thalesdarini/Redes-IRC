#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>

#define PORT 60001
#define SERVER_IP "127.0.0.1"



int main(void){
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
    

    char recv_message[4097], sent_message[100000], parc_message[4097];
    
    //loop for continuos communication
    while (1){
        // Clean buffers:
        memset(sent_message,'\0',sizeof(sent_message));

        // Get input from the user:
        printf("Enter message: ");
        fgets(sent_message, 100000, stdin);
        int tam_message = (strlen(sent_message)/4096) + 1;

        int exit = 0; //indicate if one of the user wants to end the connection 

        if (strcmp("\\quit\n", sent_message) == 0){ //verify if the client typed "/quit"
            exit = 1;
        }

        // Send the size of message to size server:
        if(send(socket_desc, &tam_message , sizeof(tam_message), 0) < 0){
            printf("Unable to send size\n");
            return -1;
        }

        //Send each part of message to server
        int ok = 1;
        for (int i = 0; i < tam_message; i++){
            memset(parc_message,'\0',sizeof(parc_message));
            strncpy(parc_message,sent_message+(i*4096), 4096);
            if(send(socket_desc, parc_message, strlen(parc_message), 0) < 0){
                printf("Unable to send message\n");
                return -1;
            }

            //confirm if another msg can be sent
            if(recv(socket_desc, &ok, sizeof(ok), 0) < 0){
                printf("Error while receiving server's confirmation\n");
                return -1;
            } 

        }
        printf("\n");

        if (exit) //if the user wants to end the communication => close socket
            break;

        // Receive the server's message size:
        if(recv(socket_desc, &tam_message, sizeof(tam_message), 0) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }

        printf("Server's message:\n");

        //Receive each part of server's message
        for (int i = 0; i < tam_message; i++){
            memset(recv_message,'\0',sizeof(recv_message));
            if (recv(socket_desc, recv_message, sizeof(recv_message), 0) < 0){
                printf("Couldn't receive\n");
                return -1;
            }
            printf("%s", recv_message);

            //send confirmation to receive another msg
            if (send(socket_desc, &ok, sizeof(ok), 0) < 0){
                printf("Error while sending confirmation\n");
                return -1;
            }

        }
        printf("\n");

        if (strcmp("\\quit\n", recv_message) == 0) //verify if server wants to end communication
            break;
    }
    

    // Close the socket:
    close(socket_desc);
    
    return 0;
}
