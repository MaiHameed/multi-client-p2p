#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/time.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>                                                                         
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Header file that specified PDU struct
#include "ds.h"

#define	BUFLEN      100     // Max 100 bytes per packet

// Globals
char peerName[11];                          // Holds the user name, same name per instance of client

// Necessary for UDP connection
char	*host = "localhost";                // Default host
int		port = 3000;                        // Default port                
int		s_udp, s_tcp, new_tcp, n, type;	    // socket descriptor and socket type	
struct 	hostent	*phe;	                    // pointer to host information entry	

struct	sockaddr_in server, client;         // To generate a TCP connection
struct 	sockaddr_in content_server_sin;
int     tcp_host, tcp_port;                 // The generated TCP host and port into easier to call variables

void printTasks(){
    printf("Choose a task:\n");
    printf("    R: Register content to the index server\n");
    printf("    T: De-register content\n");
    printf("    D: Download content\n");
    printf("    O: List all the content available on the index server\n");
    printf("    L: List all local content registered\n");
    printf("    Q: To de-register all local content from the server and quit\n");
}

int fileTransfer(int tcp){
    // TODO: Implement local file transfer over TCP
    exit(0); // Success and exit
}

void registerContent(){
    struct pduR packetR;                    // The PDU packet to send to the index server
    struct pdu sendPacket;

    int validContentName = 0;               // Flag to check validity of user provided content name
    int acknowledgedFromIndex = 0;          // Flag to verify acknowledgement from index server
    char input[101];                        // Temp placeholder for user input
    char writeMsg[101];                     // Temp placeholder for outgoing message to index server
    int readLength;                         // Length of incoming data bytes from index server
    char readPacket[101];                   // Temp placeholder for incoming message from index server

    /*
        The below while loop encloses the generation and sending of an R type data packet
        from the peer to the server. The while loop exits after receiving acknowledgement
        from the server
    */
    while(!acknowledgedFromIndex){
        // The below check will ensure a valid user input for the content name
        printf("Enter a valid content name, 9 characters or less:\n");
        while(!validContentName){
            scanf("%s", input);         // Get user input for content name
            if(strlen(input) < 10){
                validContentName = 1;   // A valid content name is less than 10 characters
            }else{
                printf("Content name must be 9 characters or less, try again\n");
            }
        }

        // Build R type PDU to send to index server
        memcpy(packetR.contentName, input, 10);
        memcpy(packetR.peerName, peerName, 10);
        sprintf(packetR.host, "%d", tcp_host);
        sprintf(packetR.port, "%d", tcp_port);
        packetR.type = 'R';

        fprintf(stderr, "Built the following R PDU packet:\n");
        fprintf(stderr, "    Type: %c\n", packetR.type);
        fprintf(stderr, "    Peer Name: %s\n", packetR.peerName);
        fprintf(stderr, "    Content Name: %s\n", packetR.contentName);
        fprintf(stderr, "    Host: %s\n", packetR.host);
        fprintf(stderr, "    Port: %s\n", packetR.port);
        fprintf(stderr, "\n");

        // Parse the pduR type into default pdu type for transmission
        // sendPacket.data = [peerName]+[contentName]+[host]+[port]
        sendPacket.type = packetR.type;
        strcpy(sendPacket.data, packetR.peerName);
        strcat(sendPacket.data, packetR.contentName);
        strcat(sendPacket.data, packetR.host);
        strcat(sendPacket.data, packetR.port);
        fprintf(stderr, "Parsed the R type PDU into the following general PDU:\n");
        fprintf(stderr, "    Type: %c\n", sendPacket.type);
        fprintf(stderr, "    Data: %s\n", sendPacket.data);
        fprintf(stderr, "\n");

        // Sends the data packet to the index server
        write(s_udp, &sendPacket, BUFLEN);     
        
        // Wait for message from the server and check the first byte of packet to determine the PDU type (A or E)
        readLength = read(s_udp, readPacket, BUFLEN);
        int i = 1;
        struct pduE packetE;                    // Potential responses from the index server
        struct pduA packetA;
        switch(readPacket[0]){       
            case 'E':
                // Copies incoming packet into a PDU-E struct
                packetE.type = readPacket[0];
                while(readPacket[i] != '\0'){ 
                    packetE.errMsg[i-1] = readPacket[i];
                    i++;
                }

                // Output to user
                printf("Error registering content:\n");
                printf("    %s:\n", packetE.errMsg);
                printf("\n");
                break;
            case 'A':
                // Copies incoming packet into a PDU-A struct
                packetA.type = readPacket[0];
                while(readPacket[i] != '\0'){ 
                    packetA.peerName[i-1] = readPacket[i];
                    i++;
                }

                // Output to user
                printf("The following content has been successfully registered:\n");
                printf("    Peer Name: %s\n", packetA.peerName);
                printf("    Content Name: %s\n", packetR.contentName);
                printf("    Host: %s\n", packetR.host);
                printf("    Port: %s\n", packetR.port);
                printf("\n");
                acknowledgedFromIndex = 1;
                break;
            default:
                printf("Unable to read incoming message from server\n\n");
        }
    }
}

int main(int argc, char **argv){
    
    // Checks input arguments and assigns host:port connection to server
    switch (argc) {
		case 1:
			break;
		case 2:
			host = argv[1];
            break;
		case 3:
			host = argv[1];
			port = atoi(argv[2]);
			break;
		default:
			fprintf(stderr, "usage: UDP Connection to Index Server [host] [port]\n");
			exit(1);
	}

    struct 	sockaddr_in sin;                    // UDP connection

    // Generate a UDP connection to the index server
    // Map host name to IP address, allowing for dotted decimal 
	if ( phe = gethostbyname(host) ){
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	}
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");                                                               
    // Allocate a socket 
	s_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if (s_udp < 0)
		fprintf(stderr, "Can't create socket \n");                                                                
    // Connect the socket 
	if (connect(s_udp, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		fprintf(stderr, "Can't connect to %s\n", host);
	}

    // Generate a TCP connection
    // Create a TCP stream socket	
	if ((s_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Can't create a TCP socket\n");
		exit(1);
	}
	content_server_sin.sin_family = AF_INET;
	content_server_sin.sin_port = htons(0);
	content_server_sin.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s_tcp, (struct sockaddr *)&content_server_sin, sizeof(content_server_sin));
	
	socklen_t sin_len = sizeof(content_server_sin);
	if (getsockname(s_tcp, (struct sockaddr *)&content_server_sin, &sin_len) == -1) {
		fprintf(stderr, "Can't get TCP socket name %d\n", s_tcp);
		exit(1);
	}
    tcp_host = content_server_sin.sin_addr.s_addr;
    tcp_port = content_server_sin.sin_port;
    fprintf(stderr, "Successfully generated a TCP socket\n");
	fprintf(stderr, "TCP socket address %d\n", tcp_host);
	fprintf(stderr, "TCP socket port %d\n", tcp_port);
    
    printf("=============================\n");
    printf("Welcome to the P2P Network!\n");
	printf("=============================\n");
    
    // TODO: Handle edge case for when username matches another in the server, should be handled by
    // the registration component
    printf("Please enter a username:\n");
    while(read (0, peerName, sizeof(peerName)) > 10){
        printf("User name is too long, please keep it 9 characters or less:\n");
    }
    // Change last char to a null termination instead of newline, for purely aesthetic formatting purposes only
    peerName[strlen(peerName)-1] = '\0'; 
    printf("Welcome %s!\n\n", peerName);

    // Main control loop
    char userChoice[2];
    int quit = 0;
    fd_set rfds, afds;     
    while(!quit){
        // Prompt user to select task
        printTasks();

        // Checks which stream to listen to: stdin (user), TCP (peer)
        // TODO: Not sure about this chunk
        /*
        memcpy(&rfds, &afds, sizeof(rfds));
        FD_ZERO(&afds);
        FD_SET(s_tcp, &afds);

        int maxfd = (s_tcp > 0) ? s_tcp : 0;
        if (select(maxfd + 1, &rfds, NULL, NULL, NULL) == -1) {
			fprintf(stderr, "Error during select system call\n");
		}

        // Check if any TCP sockets has pending data
        if(FD_ISSET(s_tcp, &rfds)){
            fprintf(stderr, "Detected a TCP socket connection incoming\n");
            // TODO Implement management of TCP connections. These will be peers requesting data
            int client_len = sizeof(client);
            new_tcp = accept(s_tcp,(struct sockaddr*)&client, &client_len);
            if(new_tcp < 0){
                fprintf(stderr, "Can't accept incoming TCP connection\n");
                exit(1);
            }
            switch(fork()){
                case 0: // Child
                    (void) close(s_tcp);
                    exit(fileTransfer(new_tcp));
                default: // Parent
                    (void) close(new_tcp);
                case -1:
                    fprintf(stderr, "Error during the fork\n");
                    exit(1);
            }
        }
        */

        read(0, userChoice, 1);
        fprintf(stderr, "Read user input\n");

        // Perform task
        switch(userChoice[0]){
            case 'R':
                registerContent();
                break;
            case 'D':
                break;
            case 'T':
                break;
            case 'O':
                break;
            case 'Q':
                // TODO: Implement de-registration of all content and quit
                quit = 1;
                break;
            default:
                printf("Invalid choice, try again\n");
        }  
    }
    close(s_udp);
    close(s_tcp);
}