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

// User defined files
#include "ds.h"

#define	BUFLEN      3000     // Max bytes per packet

// Globals
char peerName[10];                          // Holds the user name, same name per instance of client
char localContentName[5][10];               // Creates an array of strings that represent the name of the local content that has been registered
                                            // via the index server. Can store 5 elements of 10 bytes per content (9 + null terminating char)
char localContentPort[5][6];                // Stores the port number associated with each piece of locally registered content
int numOfLocalContent = 0;                  // Stores number of saved local content

// UDP connection variables
char	*host = "localhost";                // Default host
int		port = 3000;                        // Default port                
int		s_udp, s_tcp, new_tcp, n, type;	    // socket descriptor and socket type	
struct 	hostent	*phe;	                    // pointer to host information entry	

// TCP connection variables        
struct 	sockaddr_in content_server_sin;
int     tcp_host, tcp_port;                 // The generated TCP host and port into easier to call variables

void addToLocalContent(char contentName[], char port[]){
    strcpy(localContentName[numOfLocalContent], contentName);
    strcpy(localContentPort[numOfLocalContent], port);
    
    // stderr Logging purposes only
    fprintf(stderr, "Added the following content to the local list of contents:\n");
    fprintf(stderr, "   Content Name: %s\n", localContentName[numOfLocalContent]);
    fprintf(stderr, "   Port: %s\n", localContentPort[numOfLocalContent]);

    numOfLocalContent++;
}

void removeFromLocalContent(char contentName[]){
    // TODO: Find in array list and remove from both lists, decrement numOfLocalContent
    int j;
    int foundElement = 0;

    // This for loop parses the local list of content names for the requested content name
    for(j = 0; j < numOfLocalContent; j++){
        if(strcmp(localContentName[j], contentName)){
            // the requested name was found in the list, proceed to delete element in both name and port lists
            printf("The following content was deleted:\n");
            printf("    Content Name: %s\n", localContentName[j]);
            printf("    Port: %s\n", localContentPort[j]);

            // Delete element
            strcpy(localContentName[j], '\0');
            strcpy(localContentPort[j], '\0');

            // This loop moves all elements underneath the one deleted up one to fill the gap
            while(j < numOfLocalContent - 1){
                strcpy(localContentName[j], localContentName[j+1]);
                strcpy(localContentPort[j], localContentPort[j+1]);
            }
            foundElement = 1;
            break;
        }
    }

    // Sends an error message to the user if the requested content was not found
    if(!foundElement){
        printf("Error, the requested content name was not found in the list:\n");
        printf("    Content Name: %s\n", contentName);
    }
}

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
    char input[101];                        // Temp placeholder for user input
    char writeMsg[101];                     // Temp placeholder for outgoing message to index server
    int readLength;                         // Length of incoming data bytes from index server
    char readPacket[101];                   // Temp placeholder for incoming message from index server

    /*
        The below while loop encloses the generation and sending of an R type data packet
        from the peer to the server. The while loop exits after receiving acknowledgement
        from the server
    */
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
    memset(&sendPacket, '\0', sizeof(sendPacket));          // Sets terminating characters to all elements
    int dataOffset = 0;

    sendPacket.type = packetR.type;
    memcpy(sendPacket.data + dataOffset, 
            packetR.peerName, 
            sizeof(packetR.peerName));
    dataOffset += sizeof(packetR.peerName);
    memcpy(sendPacket.data + dataOffset, 
            packetR.contentName,
            sizeof(packetR.contentName));
    dataOffset += sizeof(packetR.contentName);
    memcpy(sendPacket.data + dataOffset, 
            packetR.host,
            sizeof(packetR.host));
    dataOffset += sizeof(packetR.host);
    memcpy(sendPacket.data + dataOffset, 
            packetR.port,
            sizeof(packetR.port));

    fprintf(stderr, "Parsed the R type PDU into the following general PDU:\n");
    fprintf(stderr, "    Type: %c\n", sendPacket.type);
    fprintf(stderr, "    Data: %s\n", sendPacket.data);
    int m;
    for(m = 0; m <= sizeof(sendPacket.data)-1; m++){
        fprintf(stderr, "%d: %c\n", m, sendPacket.data[m]);
    }
    fprintf(stderr, "\n");

    // Sends the data packet to the index server
    write(s_udp, &sendPacket, sizeof(sendPacket.type)+sizeof(sendPacket.data));     
    
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
            addToLocalContent(packetR.contentName, packetR.port);
            break;
        default:
            printf("Unable to read incoming message from server\n\n");
    }
}

void deregisterContent(char contentName[]){
    struct pduT packetT;
    struct pdu sendPacket;

    //  Build the T type PDU
    packetT.type = 'T';
    memcpy(packetT.peerName, peerName, sizeof(peerName));
    memcpy(packetT.contentName, contentName, sizeof(packetT.contentName));

    // Parse the T type into a general PDU for transmission
    // sendPacket.data = [peerName]+[contentName]+[host]+[port]
    memset(&sendPacket, '\0', sizeof(sendPacket));          // Sets terminating characters to all elements
    int dataOffset = 0;

    sendPacket.type = packetT.type;
    memcpy(sendPacket.data + dataOffset, 
            packetT.peerName, 
            sizeof(packetT.peerName));
    dataOffset += sizeof(packetT.peerName);
    memcpy(sendPacket.data + dataOffset, 
            packetT.contentName,
            sizeof(packetT.contentName));
    
    // stderr output, log purposes only
    fprintf(stderr, "Parsed the T type PDU into the following general PDU:\n");
    fprintf(stderr, "    Type: %c\n", sendPacket.type);
    fprintf(stderr, "    Data: %s\n", sendPacket.data);
    int m;
    for(m = 0; m <= sizeof(sendPacket.data)-1; m++){
        fprintf(stderr, "%d: %c\n", m, sendPacket.data[m]);
    }
    fprintf(stderr, "\n");

    // Sends the data packet to the index server
    write(s_udp, &sendPacket, sizeof(sendPacket.type)+sizeof(sendPacket.data));
    // Removes the content from the list of locally registered content
    removeFromLocalContent(packetT.contentName);
}

void listLocalContent(){
    int j;

    printf("List of names of the locally registered content:\n");
    printf("Num\t\tName\t\tPort\n");
    for(j = 0; j < numOfLocalContent; j++){
        printf("%d\t\t%s\t\t%s\n", j, localContentName[j], localContentPort[j]);
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

    struct 	sockaddr_in sin;            

    memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                                                                
	sin.sin_port = htons(port);

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
    bzero((char *)&content_server_sin, sizeof(struct sockaddr_in));
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
    printf("Welcome %s! ", peerName);

    // Main control loop
    char userChoice[2];
    char userInput[10];
    int quit = 0;
    int j;
    fd_set rfds, afds;   

    while(!quit){
        // Prompt user to select task
        printTasks();

        // Check if there is an incoming TCP connection
        /*
        listen(s_tcp, 5); // queue up to 5 connect requests  

        if(true){
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

        read(0, userChoice, 2);

        // Perform task
        switch(userChoice[0]){
            case 'R':   // Register content to the index server
                registerContent();
                break;
            case 'T':   // De-register content
                read(0, userInput, 10);
                deregisterContent(userInput);
                break;
            case 'D':   // Download content
                break;
            case 'O':   // List all the content available on the index server
                break;
            case 'L':   // List all local content registered
                listLocalContent();
                break;
            case 'Q':   // De-register all local content from the server and quit
                for(j = 0; j < numOfLocalContent; j++){
                    deregisterContent(localContentName[j]);
                }
                quit = 1;
                break;
            default:
                printf("Invalid choice, try again\n");
        }  
    }
    close(s_udp);
    close(s_tcp);
    exit(0);
}