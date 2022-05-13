#pragma once
/* 
 * glue.h --- max buffer size
 * 
 * Author: Stephen Taylor
 * Created: 05-15-2021
 * Version: 4.0
 * 
 * 3.8->4.0 Removed sendbuff from server and fixed usage statements
 */
#define _DEFAULT_SOURCE
#include <stdio.h>		/* printf */
#include <stdlib.h> 		/* EXIT_FAILURE & EXIT_SUCCESS */
#include <stdbool.h>	/* booleans */
#include <string.h>		/* memset */
#include <inttypes.h>	/* uint8_t */
#include <unistd.h>		/* close */
#include <errno.h>
#include <time.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>		/* socket calls */
#include <sys/time.h>
#include <netinet/in.h>		/* htons & inet_addr */
#include <arpa/inet.h>		/* htons & inet_addr */

#define VERSION "4.3"
#define IPLEN 16
#define UDP_PORT 9001
#define MAXMSG (2048*1024)	/* 2MBbyte */
#define ACK 0x80
#define NAK 0x90
#define CHUNK_SIZE	(1024*16)
#define KEYLEN 64

static uint8_t WSSOF[KEYLEN] = {0x66, 0x55, 0x99, 0xAA, 0xAA, 0x99, 0x55, 0x66,
	                          0xAA, 0x99, 0x55, 0x66, 0x66, 0x55, 0x99, 0xAA,
	                          0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
	                          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	                          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
														0xAA, 0x99, 0x55, 0x66, 0x66, 0x55, 0x99, 0xAA,
	                          0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
	                          0x66, 0x55, 0x99, 0xAA, 0xAA, 0x99, 0x55, 0x66};
static uint8_t WSEOF[KEYLEN] = {0x66, 0x55, 0x99, 0xAA, 0xAA, 0x99, 0x55, 0x66,
	                          0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
														0xAA, 0x99, 0x55, 0x66, 0x66, 0x55, 0x99, 0xAA,
	                          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	                          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	                          0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
	                          0xAA, 0x99, 0x55, 0x66, 0x66, 0x55, 0x99, 0xAA,
														0x66, 0x55, 0x99, 0xAA, 0xAA, 0x99, 0x55, 0x66};

static struct timeval tv;				/* timeout used at client */
static bool verbose;						/* true if verbose printing */
static bool ack;								/* true if acks are in use */

typedef struct response_struct {
	uint8_t type;
} response_t;
	
static void errorExit(char *str) {
	printf("%s",str);
	exit(EXIT_FAILURE);
}

static void msgprint(char *txt,uint8_t *bp,uint64_t len) {
	uint64_t i;
	printf("[%s: ",txt);
	if(len>10) {
		for(i=0; i<4; i++)
			printf("%02"PRIx8"",*(bp+i));
		printf("...");
		for(i=len-4; i<len; i++)
			printf("%02"PRIx8"",*(bp+i));
	}
	else {
		for(i=0; i<len; i++)
			printf("%02"PRIx8"",*(bp+i));
	}
	printf(" (%"PRIu64" bytes)]\n",len);
}


/* gets a response from a server */
static void getresponse(int sock) {
  int64_t len;
  response_t reply;
  struct sockaddr_in sockaddr;
 	socklen_t socklen;

  reply.type = 0x00;
  if (verbose) printf("[waiting for response]\n");

  len = recvfrom(sock,
  	             (void*)&reply,
  	             (uint64_t)sizeof(response_t),
  	             0,
  	             (struct sockaddr*)&sockaddr,
  	             &socklen);

  if (len<0) {
    printf("[timed out: no response]\n");
  } else {
    switch(reply.type) {
      case ACK:											/* move on and keep quiet */
			  printf("[delivered]\n");
			  break;
		  case NAK:
			  printf("[NAK]\n");
			  break;
		  default:
			  printf("[invalid response: %02" PRIx8 "]\n",reply.type);
			  exit(EXIT_FAILURE);
		}
	}
}


/* gets a response from a server */
static void sendresponse(int sock, uint8_t* buffer, uint64_t len, struct sockaddr_in *servaddr) {
  ssize_t nsent;
  nsent = sendto(sock, (void*)buffer, len, 0, (struct sockaddr*)servaddr,sizeof(struct sockaddr_in) );
  if (nsent < 0) {
    printf("Failed to send response\n");    	
  } else {
   	printf("Sent response\n");
 	}
}


/*
 * msgsend -- send a message of a given length out of buffer 
 * returns 0 if successful; nonzero otherwise
*/
static int msgsend(int sock, uint8_t *buffer, uint64_t len, struct sockaddr_in *servaddr) {
  uint8_t *bp;
  ssize_t nsent;
	size_t sendsize;
	uint64_t length;

	bp=buffer;
	length=len;
  while(length>0) {
		if (length>CHUNK_SIZE)
			sendsize=CHUNK_SIZE;
		else
			sendsize=length;

    nsent = sendto(sock, (void*)bp, sendsize, 0, (struct sockaddr*)servaddr,sizeof(struct sockaddr_in) );

    if (nsent < 0) {
      printf("Failed to send %ld bytes\n", sendsize);    	
			return -1;
    } else {
    	printf("Sent %ld bytes\n", sendsize);
 			if (ack) {
 				getresponse(sock);
 			} else {
 				sleep(1);
 			}
    }
    bp += nsent;
    length -= nsent;
  }

	if(verbose)
		msgprint("sent",buffer,len);
	return 0;
}


/* 
 * recvall -- recieve all of a potentially very long (1GB) message
		len = msgrecv(client_sock,(void*)recvbuff);
 */
static int64_t msgrecv(int sock, uint8_t *buffer, uint64_t bufsize, struct sockaddr_in *servaddr) {
  uint8_t *bp;
  int64_t length; 				/* buffer space remaining and recvd so far */
  ssize_t nrecv;								/* number of bytes recived in one call */
	socklen_t socklen;

  bp = buffer;				/* where to place the bytes */
	length = 0;
	if((nrecv = recvfrom(sock,(void*)bp,bufsize,0,(struct sockaddr*)servaddr,&socklen))>0) {
		bp += nrecv;
		length += nrecv;
	}

	if (nrecv<0)
		return nrecv;

	if (verbose)
		msgprint("recd",buffer,length);

	return length;
}


/* 
 * makesock() -- makes a socket.
 */
static int makesock(struct sockaddr_in *addrp,char *ip, uint16_t port) {
	int sock;

	/* set up the server address */
  memset(addrp, 0, sizeof(struct sockaddr_in));
  addrp->sin_family = AF_INET;
  addrp->sin_port   = htons(port);
	if(ip==NULL)
		addrp->sin_addr.s_addr = htonl(INADDR_ANY);
	else if(inet_aton(ip,&(addrp->sin_addr)) <= 0)
    errorExit("error on inet_aton\n");

  /*  Create a reusable UDP socket with a timeout  */
  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 )
    errorExit("server: error creating listening socket.\n");

	return sock;
}

