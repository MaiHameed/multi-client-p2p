#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Header file that specified PDU struct
#include "ds.h"

#define	BUFLEN      100     // Max 100 bytes per packet

/*
Things that a client/peer can do:
1 - Content Registration
2 - Content Download
3 - Content Listing
4 - Content De-registration
5 - Quit
*/
void printTasks(){
    printf("Choose a task:\n");
    printf("    R: Register content to the index server\n");
    printf("    D: Download content\n");
    printf("    T: De-register content\n");
    printf("    O: List all the content available on the index server\n");
    printf("    Q: To de-register all content from the server and quit as a peer\n");
}

void registerContent(int s){
    struct pduR packetR;
    int validPeerName = 0;
    int validContentName = 0;
    int acknowledgedFromIndex = 0;
    char input[101];
    char writeMsg[101];
    int readLength;

    printf("TODO: Implement opening a TCP socket for content downloading HERE\n");
    /*
        The below while loop encloses the generation and sending of an R type data packet
        from the peer to the server. The while loop exits after receiving acknowledgement
        from the server
    */
    while(!acknowledgedFromIndex){
        // The below check will ensure a valid user input for the peer name
        printf("Enter a valid peer name, 9 characters or less:\n");
        while(!validPeerName){
            scanf("%s", input);         // Get user input for peer name
            if(strlen(input) < 10){
                validPeerName = 1;      // A valid peer name is less than 10 characters
            }else{
                printf("Peer name must be 9 characters or less, try again\n");
            }
        }
        memcpy(packetR.peerName, input, 10);

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
        memcpy(packetR.contentName, input, 10);
        packetR.type = 'R';

        fprintf(stderr, "Packet Type: %c\n", packetR.type);
        fprintf(stderr, "Packet peerName: %s\n", packetR.peerName);
        fprintf(stderr, "Packet contentName: %s\n", packetR.contentName);

        // Sends the R data packet to the index server
        write(s, &packetR, BUFLEN);     
        
        // Wait for message from the server and check the first byte of packet to determine the PDU type (A or E)
        readLength = read(s, input, BUFLEN);
        switch(input[0]){       
            case 'E':
                struct pduE packetE;
                int i = 1;

                // Copies incoming packet into a PDU-E struct
                packetE.type = input[0];
                while(input[i] != '\0'){ 
                    packetE.errMsg[i-1] = input[i];
                }

                // Output to user
                printf("Error registering content:\n");
                printf("    %s:\n", packetE.errMsg);
                break;
            case 'A':
                struct pduA packetA;
                int i = 1;

                // Copies incoming packet into a PDU-A struct
                packetA.type = input[0];
                while(input[i] != '\0'){ 
                    packetA.peerName[i-1] = input[i];
                }

                // Output to user
                printf("The following content has been successfully registered:\n");
                printf("    Peer Name: %s\n", packetA.peerName);
                acknowledgedFromIndex = 1;
                break;
            default:
                printf("Unable to read incoming message from server\n");
        }
    }
}

int main(int argc, char **argv){
    
    // Variables for connecting to the index server
    char	*host = "localhost";                // Default host
    int		port = 3000;                        // Default port
    struct 	sockaddr_in sin;                    // An internet endpoint address
    int		s, n, type;	                        // socket descriptor and socket type	
    struct 	hostent	*phe;	                    // pointer to host information entry	

    char    userChoice;                         // Represents user selection at application runtime
    
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
    
    // Map host name to IP address, allowing for dotted decimal 
	if ( phe = gethostbyname(host) ){
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	}
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    // Allocate a socket 
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	                                                                      
    // Connect the socket 
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		fprintf(stderr, "Can't connect to %s\n", host);
	}
    
    printf("=============================\n");
    printf("Welcome to the P2P Network!\n");
	printf("=============================\n");
    
    // Main control loop
    while(1){
        // Prompt user to select task
        printTasks();
        userChoice = getchar();
        
        // Perform task
        switch(userChoice){
            case 'R':
                registerContent(s);
                break;
            case 'D':
                break;
            case 'T':
                break;
            case 'O':
                break;
            case 'Q':
                break;
        }
    }
}