#include "./ds.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include "uthash/src/utarray.h"
#include "./set.h"

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

struct tpacket {
  char type;
  char data[100];
};

// Index System Data Structures
typedef struct  {
  char contentName[10]; 
  char peerName[10];
  int host;
  int port;
} contentListing;

int main(int argc, char *argv[]) 
{
  // Packets
  struct  pduR packetR, *pktR;
  struct  pduS packetS;
  struct  pduO packetO;
  struct  pduT packetT;
  struct  pduE packetE;
  struct  pduA packetA;

  // Transmission Variables
  struct  tpacket packetrecieve;
  int     r_host, r_port;

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
  set_init(&contentSet);

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
      if (recvfrom(s, &packetrecieve, BUFLEN, 0,(struct sockaddr *)&fsin, &alen) < 0)
			  fprintf(stderr, "[ERROR] recvfrom error\n");
      fprintf(stderr, "Msg Recieved\n");
      switch(packetrecieve.type){
        /* Peer Server Content Registration */
        case 'R':
          memset(&packetR,'\0', sizeof(packetR));
          for(i=0;i<10;i++){
              packetR.peerName[i] = packetrecieve.data[i]; // 0:9
              packetR.contentName[i] = packetrecieve.data[i+10]; // 10:19
              if (i<6) {
                packetR.host[i] = packetrecieve.data[i+20]; // 20:25
              }
              if (i<5) {
                packetR.port[i] = packetrecieve.data[i+26]; // 26:31
              }
          }
          r_host = atoi(packetR.host);
          r_port = atoi(packetR.port);

          contentListing packet, *pkt, *pktr;
        
          // Content and user validation
          for(pktr=(contentListing*)utarray_front(contentList);
              pktr!=NULL;
              pktr=(contentListing*)utarray_next(contentList,pktr)) {
                // Has the same peername has uploaded the same thing before?
                if (pktr->peerName == packetR.peerName && pktr->contentName == packetR.contentName) {
                  errorFlag=1;
                  break;
                }
                // Is this peername being used by someone else?
                // if ((strcmp(pktr->peerName,packetR.peerName) == 0) && (pktr->host != packetR.host)) {
                //   errorFlag=2;
                //   break;
                // }
          }

          /*  Send Reply  */
          memset(&packet,'\0', sizeof(packet));

          if (errorFlag==0){
            // Commit Content
            set_add(&contentSet,packet.contentName);
            
            strcpy(packet.contentName,packetR.contentName);
            strcpy(packet.peerName,packetR.peerName);
            packet.host = r_host;
            packet.port = r_port;
            utarray_push_back(contentList, &pkt);

            fprintf(stderr, "Peer Name: %s\n", packet.peerName);
            fprintf(stderr, "Content Name: %s\n", packet.contentName);
            fprintf(stderr, "Port %d, Host %d\n", packet.port, packet.host);
            
            // Send Ack Packet
            packetA.type = 'A';
            memset(packetA.peerName, '\0', 10);
            strcpy(packetA.peerName, packet.peerName);
            sendto(s, &packetA, BUFLEN, 0,(struct sockaddr *)&fsin, sizeof(fsin));
          }
          else if (errorFlag=2) {
            // Send Err Packet
            packetE.type = 'E';
            memset(packetE.errMsg, '\0', 100);
					  strcpy(packetE.errMsg,"PeerName has already been registered");
					  sendto(s, &packetE, BUFLEN, 0,(struct sockaddr *)&fsin, sizeof(fsin));
          }
          else {
            // Send Err Packet
            packetE.type = 'E';
            memset(packetE.errMsg, '\0', 100);
					  strcpy(packetE.errMsg,"File Already Registered Error");
					  sendto(s, &packetE, BUFLEN, 0,(struct sockaddr *)&fsin, sizeof(fsin));
          }
          
          break;
        
        /* Peer Server Content Location Request */
        case 'S':
          break;
        
        /* Peer Server Content List Request */
        case 'O':
          // We don't care about the data in this one
          break;
        
        /* Peer Server Content De-registration */
        case 'T':
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