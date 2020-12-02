#include "./ds.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>
#include "klib/khash.h"
#include "uthash/src/uthash.h"
#include "uthash/src/utarray.h"
#include "set.h"

/*
Storage
Registration -> Peername (Internal Ref) needs to be saved with address
Download -> Get Peername based on content (least used)
List -> Retrieve list of ALL distinct content

De-Registration -> Content pulled based on Peername

From This -> Get This
-----------------------------------
Peername -> Address (IP + Port)

Content -> [Peernames]

Peername -> [Content] DONT NEED 

[Content] (unique)

*/


#define BUFLEN		101			/* buffer length */

struct pdu {
	char type;
	char data[100];
};

// Index System Data Structures
typedef struct  {
  char contentName[10];   // (Key) Content Name
  char peerName[10];
  char address[80];
} contentListing;

int main(int argc, char *argv[]) 
{
  // Packets
  struct  pduR packetR;
  struct  pduS packetS;
  struct  pduO packetO;
  struct  pduT packetT;
  struct  pduE packetE;
  struct  pduA packetA;
  char    packetrecieve[101];

  // Socket Primitives
  struct  sockaddr_in fsin;	        /* the from address of a client	*/
	char    *pts;
	int		  sock;			                /* server socket */
	time_t	now;			                /* current time */
	int		  alen;			                /* from-address length */
	struct  sockaddr_in sin;          /* an Internet endpoint address */
  int     s,sz, type;               /* socket descriptor and socket type */
	int 	  port = 3000;              /* default port */
	int 	  counter, n,m,i,bytes_to_read,bufsize;
  int     errorFlag = 0;

  // Index System
  UT_icd contentListing_icd = {sizeof(contentListing), NULL, NULL, NULL};
  UT_array *contentList;
  utarray_new(contentList,&contentListing_icd);
  SimpleSet contentSet;

	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(1);
	}
    /* Complete the socket structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;         //it's an IPv4 address
    sin.sin_addr.s_addr = INADDR_ANY; //wildcard IP address
    sin.sin_port = htons(port);       //bind to this port number
                                                                                                 
    /* Allocate a socket */
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
		fprintf(stderr, "can't create socket\n");
                                                                                
    /* Bind the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
    listen(s, 5);	
	  alen = sizeof(fsin); 
    
    while(1) {
      if (recvfrom(s, &packetrecieve, BUFLEN+1, 0,(struct sockaddr *)&fsin, &alen) < 0)
			  fprintf(stderr, "[ERROR] recvfrom error\n");
      fprintf(stderr, "Msg Recieved\n");
      switch(packetrecieve[0]){

        /* Peer Server Content Registration */
        case 'R':

          packetR.type = packetrecieve[0];
          for(i=0;i<80;i++){
            if (i<10){ 
              packetR.peerName[i] = packetrecieve[i+1];
              packetR.contentName[i] = packetrecieve[i+11];
            }
            packetR.address[i] = packetrecieve[i+21];
          }
          contentListing packet, *pkt, *pktr;
        
          // Content and user validation
          for(pktr=(contentListing*)utarray_front(contentList);
              pktr!=NULL;
              pktr=(contentListing*)utarray_next(contentList,pktr)) {
                // Content duplicate check
                if (pktr->peerName == packetR.peerName && pktr->contentName == packetR.contentName) {
                  // Send Error Packet
                  packetE.type = 'E';
					        strcpy(packetE.errMsg,"File Already Registered Error");
					        sendto(s, &packetE, BUFLEN, 0,(struct sockaddr *)&fsin, sizeof(fsin));
                  errorFlag=1;
                  break;
                }
                // Improper Peer Name Check
                if (pktr->peerName == packetR.peerName && !(pktr->address == packetR.address)){
                  errorFlag=2;
                  break;
                }
          }
          if (errorFlag==0){
            // Commit Content
            set_add(&contentSet,packet.contentName);
            strcpy(packet.address,packetR.address);
            strcpy(packet.contentName,packetR.contentName);
            strcpy(packet.peerName,packetR.peerName);
            utarray_push_back(contentList, &pkt);
            
            // Send Ack Packet
            packetA.type = 'A';
            strcpy(packetA.peerName, packet.peerName);
            sendto(s, &packetA, BUFLEN, 0,(struct sockadNr *)&fsin, sizeof(fsin));
          }
          else if (errorFlag=2) {
            // Send Err Packet
            packetE.type = 'E';
					  strcpy(packetE.errMsg,"PeerName has already been registered");
					  sendto(s, &packetE, BUFLEN, 0,(struct sockaddr *)&fsin, sizeof(fsin));
          }
          else {
            // Send Err Packet
            packetE.type = 'E';
					  strcpy(packetE.errMsg,"File Already Registered Error");
					  sendto(s, &packetE, BUFLEN, 0,(struct sockaddr *)&fsin, sizeof(fsin));
          }
          // fprintf(stderr, "Type: %c****\n", packetR.type);
          // fprintf(stderr, "Peer Name: %s\n", packetR.peerName);
          // fprintf(stderr, "Content Name: %s\n", packetR.contentName);
          // fprintf(stderr, "Address: %s\n", packetR.address);
          break;
        
        /* Peer Server Content Location Request */
        case 'S':
          packetS.type = packetrecieve[0];
          for(i=0;i<90;i++){
            if (i<10){ 
              packetS.peerName[i] = packetrecieve[i+1];
            }
            packetS.contentNameOrAddress[i] = packetrecieve[i+11];
          }
          fprintf(stderr, "**** Type: %c ****\n", packetS.type);
          fprintf(stderr, "Peer Name: %s\n", packetS.peerName);
          fprintf(stderr, "contentNAmeOrAddress: %s\n", packetS.contentNameOrAddress);
          break;
        
        /* Peer Server Content List Request */
        case 'O':
          packetO.type = packetrecieve[0];
          // We don't care about the data in this one
          fprintf(stderr, "**** Type: %c ****\n", packetO.type);
          break;
        
        /* Peer Server Content De-registration */
        case 'T':
          packetR.type = packetrecieve[0];
          for(i=0;i<10;i++){
              packetR.peerName[i] = packetrecieve[i+1];
              packetR.contentName[i] = packetrecieve[i+11];
          }
          fprintf(stderr, "**** Type: %c ****\n", packetR.type);
          fprintf(stderr, "Peer Name: %s\n", packetR.peerName);
          fprintf(stderr, "Content Name: %s\n", packetR.contentName);
          break;
        default:
          break;
      } 

    }
}

/*
Cases:
- Content Registration (R)
  - Error (E) 
  - Acknowledgement (A)
Search for Content and Associated Content Server (S)
  - (S) Type w/ Address Field
  - (E) Type content not avail
Content Listing (O)
  - (O) Type with list of registered contents
Content De-registration (T)
  - (A) Ack
  - (E) Error

  Note: when quitting, a bunch of T types may be sent
*/