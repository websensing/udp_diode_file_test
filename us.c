/* 
 * us.c --- A simple UDP server that just each message that arrives.
 * 
 * Author: Stephen Taylor
 * Created: 04-12-2021
 * Version: 3.9
 * 
 * Description: This server recieves each message and responds with an ACK.
 * 
 */
#include <glue.h>


static uint16_t serverport;
static uint8_t recvbuff[MAXMSG];
static char srvr_fnm[256] = {'\0'};
static FILE* srvr_fd = NULL;


int main(int argc, char **argv) {
  int sock;		/*  listening socket */
  struct sockaddr_in servaddr;  /*  socket address structure */
	int64_t len;
	bool done = false;
	bool started = false;
	int arg;
	response_t reply;

	tv.tv_sec = 0;								/* no timeout used in server -- just blocks*/
	tv.tv_usec = 0;								
	serverport=UDP_PORT;
	verbose = false;
	ack=false;										/* dont use acks */
	
	for(arg=1; arg<argc; arg++) {
		if(strcmp(argv[arg],"-h")==0) {
			printf("usage: us [-p <svr_port> -v -ack]\n");
			printf("  -ack          -- acknowledge messages with ACK\n");
			printf("  -p <svr_port> -- port to serve on (default %d)\n",serverport);
			printf("  -v            -- verbose message printing\n");
			exit(EXIT_SUCCESS);
		}
		else if(strcmp(argv[arg],"-p")==0)
			serverport=(uint16_t)atoi(argv[++arg]);
		else if(strcmp(argv[arg],"-v")==0)
			verbose=true;
		else if(strcmp(argv[arg],"-ack")==0)
			ack=true;
		else {
			printf("invalid argument: %s\n",argv[arg]);
			exit(EXIT_FAILURE);
		}
	}

	sock = makesock(&servaddr, NULL, serverport); /* never timeout */
  /*  Bind socket addresss to listening socket, and listen  */
  if (bind(sock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0 )
    errorExit("server: error calling bind\n");

	printf("UDP Server v%s: Listening on port %d...\n",VERSION,serverport);
	
	while(!done) {
		/* recv a message */
		len = msgrecv(sock,(void*)recvbuff,(uint64_t)MAXMSG,&servaddr);
		if (len >= 0) {
			msgprint("recv",recvbuff,len);
      if (!started) {
        if (memcmp(recvbuff, WSSOF, KEYLEN) == 0) {
          strcpy(srvr_fnm, (const char*)&(recvbuff[KEYLEN]));
					srvr_fd = fopen(srvr_fnm, "wb");
					if (srvr_fd == NULL) {
						errorExit("[error opening server-side file]\n");
					} else {
	        	printf("[opened file: %s\n", srvr_fnm);
					}
          started = true;
        }
      } else {
        if ((len == KEYLEN) && (memcmp(recvbuff, WSEOF, KEYLEN) == 0)) {
          done = true;
          fclose(srvr_fd);
          printf("[closed file: %s\n", srvr_fnm);
        } else {
        	fwrite( recvbuff, 1, len, srvr_fd );
        	printf("[wrote %ld bytes to file\n", len);
        }
      }
			if (ack) {
				reply.type = ACK;
				sendresponse(sock,(uint8_t*)&reply,(uint64_t)sizeof(response_t),&servaddr);
			}
		} else {
			printf("[error on recieve (%d)]\n",(int)len);
			if (ack) {
				reply.type = NAK;
				sendresponse(sock,(uint8_t*)&reply,(uint64_t)sizeof(response_t),&servaddr);
			}	/* keep on serving */
		}
  }

  if (close(sock) <0)   /* close the listening socket */
    errorExit("[error closing down socket]\n");

	printf("[done]\n");
	exit(EXIT_SUCCESS);
}

