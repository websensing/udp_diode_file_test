/* 
 * uc.c --- UDP Client 
 * 
 * Author: Stephen Taylor
 * Created: 04-12-2021
 * Version: 3.7
 * 
 * Description: Simple UDP client that simulates the action of the PFD
 * picker. Loads files from a "test" directory and sends them in a UDP
 * connection to a server. Note that the picker just needs to select
 * one file and send it, whereas this client loads and sends them all
 * one after another for testing.
 * 
 * Usage: ./uc -h -- run the client help
 */
#include <glue.h>

#define SERVER_IP "127.0.0.1"		/* use the loopback */
#define ALLFILES -1							/* sending all files */
#define TIMEOUT_SECS 3					/* the default timeout */

static char serverip[IPLEN];
static uint16_t serverport;
static uint8_t sendbuff[MAXMSG];


/* load a test vector from a file */
static long int read_file(FILE *fp,uint8_t *bp) {
	uint8_t b;
	long int cnt;
	/* read in the test to a filebuffer */
	cnt=0;
	while(fread(&b,1,1,fp)==1) {
		if(cnt<MAXMSG-1) {
			*bp++ = (uint8_t)b;
			cnt++;
		}
		else
			return -1;								/* error on load */
	}
	return cnt;
}


static void load_file(char* fnm,
	                  char* fnm_base,
	                  uint8_t* buff,
	                  uint64_t* fnmlen,
	                  uint64_t* flen) {
  uint8_t* buffptr = buff;
  /* load file & flags */
  *fnmlen = strlen(fnm_base)+1;
  /* load SOF */
  memcpy(buffptr, WSSOF, KEYLEN);
  buffptr += KEYLEN;
  /* load filename w/0x00 */
  memcpy(buffptr, fnm_base, *fnmlen);
  buffptr += *fnmlen;
  /* load file data */
  FILE* fp = NULL;
  if ((fp=fopen(fnm,"rb"))==NULL) {
  	printf("unable to open %s\n", fnm);
    exit(EXIT_FAILURE);
  }
  *flen = read_file(fp, buffptr);
  if (*flen < 0) {
    printf("unable to load %s\n", fnm);
    exit(EXIT_FAILURE);
  }
  fclose(fp);
  buffptr += *flen;
  /* load EOF */
  memcpy(buffptr, WSEOF, KEYLEN);
}


static int file_send(int sock,
	                 char* fnm,
	                 char* fnm_base,
	                 uint8_t* buff,
	                 struct sockaddr_in *servaddr) {
  uint64_t flen = 0;
  uint64_t fnmlen = 0;
  uint8_t*  buffptr = buff;
  load_file(fnm, fnm_base, buffptr, &fnmlen, &flen);

  uint64_t sendlen = fnmlen + KEYLEN;
  int ret = msgsend(sock, buffptr, sendlen, servaddr);
  if (ret == 0) {
  	printf("Sent file header\n");
  	buffptr += sendlen;
  } else {
  	printf("File header send failed\n");
  	return -1;
  }

  sendlen = flen;
  ret = msgsend(sock, buffptr, sendlen, servaddr);
  if (ret == 0) {
  	printf("Sent file data\n");
	msgprint(fnm, buffptr, sendlen);
  	buffptr += sendlen;
  } else {
  	printf("Send file data failed\n");
  	return -1;
  }

  sendlen = KEYLEN;
  ret = msgsend(sock, buffptr, sendlen, servaddr);
  if (ret == 0) {
  	printf("Sent file EOF\n");
  	buffptr += sendlen;
  } else {
  	printf("Send file EOF failed\n");
  	return -1;
  }
  return 0;
}


/* connect to the server and send a file */
static bool connect_and_send(char *ip,
	                         uint16_t port,
	                         char *label,
	                         char* label_base ) {
  struct sockaddr_in servaddr;  /*  socket address structure */
  int sock = makesock(&servaddr,ip,port); /* with a timeout */

  if (tv.tv_sec!=0 || tv.tv_usec!=0) {
    if (verbose)
      printf("[setting timeout to %d:%d]\n",(int)tv.tv_sec,(int)tv.tv_usec);
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv,sizeof(tv))<0)
	  errorExit("unable to set recv timeout\n");
  }

  int ret = file_send(sock, label, label_base, sendbuff, &servaddr);
  if (ret != 0) {
    printf("File send failed!\n");
    return false;
  }

  if (close(sock)<0)							/* close the connection */
    errorExit("error closing socket\n");

  return true;
}


int main(int argc, char **argv) {
	int arg;
	char fn[128];
	char fn_base[128];

	/* set default values */
	tv.tv_sec = TIMEOUT_SECS;								/* default to 3 second timeout */
	tv.tv_usec = 0;    /* 500*1000; for half a second */
	strcpy(serverip,SERVER_IP);
    serverport=UDP_PORT;
	verbose = false;
	ack = false;

	for(arg=1; arg<argc; arg++) {
		if(strcmp(argv[arg],"-h")==0) {
			printf("usage: ./uc [ -f <filenm> -ip <addr> <port> -t <n> -v -ack ]\n");
			printf("  -f <filenm>         -- run a single test vector\n");
			printf("  -ack                -- expect acknowlegements from server\n");
			printf("  -ip <addr> <port>   -- use server at IP <addr>:<port> (default: %s:%d)\n",
						 serverip,serverport);
			printf("  -t <sec> <ms>       -- timeout after secs+ms (default %d:%d)\n",
						 (int)tv.tv_sec,(int)tv.tv_usec/1000);
			printf("  -v                  -- verbose message printing\n");
			exit(EXIT_SUCCESS);
		}
		else if(strcmp(argv[arg],"-ip")==0) {
			strcpy(serverip,argv[++arg]);
			serverport=(uint16_t)atoi(argv[++arg]);
		}
		else if(strcmp(argv[arg],"-f")==0) {
			strcpy(fn,"./tests/");
			strcpy(fn_base,argv[++arg]);
            strcat(fn,fn_base);
			printf("Sending file %s [%s]\n", fn, fn_base);
		}
		else if(strcmp(argv[arg],"-t")==0) {
			tv.tv_sec = (time_t)atoi(argv[++arg]);
			tv.tv_usec = (time_t)(atoi(argv[++arg])*1000);
		}
		else if(strcmp(argv[arg],"-v")==0) {
			printf("[verbose]\n");
			verbose=true;
		}
		else if(strcmp(argv[arg],"-ack")==0) {
			printf("[ack]\n");
			ack=true;
		}
		else {
			printf("unknown option %s\n",argv[arg]);
			exit(EXIT_FAILURE);
		}
	}

    printf("UDP Client v%s (Server@%s:%d)\n",VERSION,serverip,(int)serverport);

    if (connect_and_send(serverip, (int)serverport, fn, fn_base))
      exit(EXIT_SUCCESS);
    else
      exit(EXIT_FAILURE);
}

