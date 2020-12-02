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

void registerContent(){
    int validPeerName = 0;
    int validContentName = 0;
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
			port = argv[2];
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
                registerContent();
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