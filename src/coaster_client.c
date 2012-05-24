/*senderprog.c - a client, datagram*/
//#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


/* the port users will be connecting to */
#define COASTERS_PORT 12000

int tag;

/* universal variables */
/* a sequence of worker ids(?)
	 job status */

typedef struct packet {
  uint32_t tag;
  uint32_t flags;
  uint32_t len;
  uint32_t hsum;
  uint32_t csum;
  //char *msg;
} packet;


// Loads host info from a hostname
int get_connection(char *name, int port){

  /* get the host info */ 
  struct addrinfo *destinfo;
  struct addrinfo hints;
  //hints.ai_flags = AI_PASSIVE;
  char str_port[6];
	sprintf(str_port, "%d", port);
  //sprintf(str_port, "%d", COASTERS_PORT);
  int gotinfo = getaddrinfo(name, str_port, NULL, &destinfo);
  if(gotinfo != 0){
    printf("got no addrinfo %d\n", gotinfo);
    return -1;
  }
  int sockfd;

  struct addrinfo *iter;
  // Iterate through all addrs until we get a good one
  for(iter = destinfo; iter != NULL; iter = iter->ai_next) {
    if((sockfd = socket(iter->ai_family, iter->ai_socktype,
			iter->ai_protocol)) == -1)
      continue;

    if(connect(sockfd, iter->ai_addr, iter->ai_addrlen) == -1){
      close(sockfd);
      continue;
    }
    
    // If we get this far, we have a good addrinfo
    break;
  }

  if(iter == NULL){
    printf("no good addrs in list\n");
    return -1;
  }

  freeaddrinfo(destinfo);
  freeaddrinfo(iter);

  return sockfd;
}

// The message should be packed into little endian format
// Based on the strace, each command is preceded by 20 bytes
// of this header
// Followed by the actual message
int pack_msg(char *msg, uint32_t tag, uint32_t flags, struct packet *buf)
{
	  buf->tag = tag;
	  buf->flags = flags;
  	buf->len = strlen(msg);
  	buf->hsum = buf->tag ^ buf->flags ^ buf->len;
		buf->csum = 0;
		//buf->msg = msg;
  	return 0;  
}

int send_header(int sockfd, void *msg){
	int numbytes = send(sockfd, msg, sizeof(struct packet), 0);
	printf("sent packet %d bytes\n", numbytes);
	return numbytes;
}
	

int send_msg(int sockfd, void *msg){
   int numbytes = send(sockfd, msg, strlen(msg), 0);
   printf("sent %d bytes\n", numbytes);
   return numbytes;
}

int pack_and_send(int sockfd, char *msg, uint32_t tag, uint32_t flags, struct packet *buf)
{
				int sent=0;
				pack_msg(msg, tag, flags, buf);
				sent += send_header(sockfd, buf);
				sent += send_msg(sockfd, msg);
				return sent;
}

int verify(char *header){
	packet recv;// = (packet) header;
	uint32_t calc = recv.tag ^ recv.flags ^ recv.len;
	if (calc == recv.hsum) return 0;
	else {
		printf("hsum error!\n");
		return 0;
	}
}

int receive_data(int sd, char *buf){
	recv(sd, buf, sizeof(buf), 0);
	if (verify(buf) < 0){
		return -1;
	}
	return recv(sd, buf, 100, 0);
}


// Configuring a channel
// Verify the things we need to send
int chan_config(int sd)
{
	struct packet *buf;
	buf = malloc(sizeof(struct packet));
	srand(time(NULL));
 	//uint32_t tag = rand() % 10000;
	uint32_t tag = 1;

	// first send a CHANNELCONFIG
	char *msg = "CHANNELCONFIG";
	int sent = pack_and_send(sd, msg, tag, 0, buf);

	// param keepalive?
	int keepalive;
	msg = " keepalive(120), reconnect";
	sent = pack_and_send(sd, msg, tag, 0, buf); 

	sent = pack_and_send(sd, "", tag, 0, buf);
	
	// assuming this is a unique channel id
	// TODO: gotta determine how this channel id is generated
	// u-[8]-[11]--8000 ????
	msg = "ub8a652a-13743a87f30--8000";
	sent = pack_and_send(sd, msg, tag, 0, buf);
  
	//send FIN packet
	msg = "";
	sent = pack_and_send(sd, msg, tag, (uint32_t) 0x00000002, buf);
	
  	
	char recvbuf[100];

	recv(sd, recvbuf, 20, 0);
	// verify(buf);
	recv(sd, recvbuf, 100, 0);
	
	//receive_data(sd, recvbuf);
	printf("received channel id: %s\n", recvbuf);

	return sent;
}

// If this command is used, we need to parse
// a config file for the params:
// jobsPerNode
// workerManager
// job / job params?
int submit_job(int sd)
{
	struct packet *buf;
	buf = malloc(sizeof(struct packet));
	srand(time(NULL));
 	uint32_t tag = rand() % 10000;
	
	// first send a CONFIGSERVICE
	char *msg = "CONFIGSERVICE";
	int sent = pack_and_send(sd, msg, tag, 0, buf);

	// param jobsPerNode
	int jobs;
	msg = "jobsPerNode=2";
	sent = pack_and_send(sd, msg, tag, 0, buf);

	// param workerManager
	msg = "workerManager=passive";
	sent = pack_and_send(sd, msg, tag, 0, buf);

	// wait to receive an OK
	char recvbuf[100];
	recv(sd, recvbuf, 20, 0);
	// verify(buf);
	recv(sd, recvbuf, 100, 0);
	if (strcmp(recvbuf, "OK") == 0){
		msg = "SUBMITJOB";
		sent = pack_and_send(sd, msg, tag, 0 buf);
	}

	// ??????????????
	// TODO: SOME LARGE FILE
	// ??????????????

	return sent;
}

/* BUILD AN SITES.XML PARSER 
   WE NEED TO BE ABLE TO PARSE THE FOLLOWING:
			URL
			PORT
			WORKER MANAGER
			JOBS PER NODE
*/


int main(int argc, char *argv[ ])
{
  if (argc != 3)
    {
      fprintf(stderr, "Client-Usage: %s <hostname> <port>\n", argv[0]);
      exit(1);
    } 

  /* connector's address information */
	/* XML */
  char *host = argv[1];
	//int port = (int) *argv[2];
	int port = 12000;
  int sockfd = get_connection(host, port);
  if(sockfd < 0){
    printf("connection error\n");
    exit(1);
  }
  
	/* TODO
	Since at this point a connection should be established
	This thing should be able to respond to messages received
	over the connection.
	Since I don't know where the coaster messages should be passed
	at the least they should be:
		1) parsed and verified
		2) the messages printed to stdout
		3) each FIN recognized with an "OK" response
  */
	
	//int sent = pack_and_send(sockfd, cmd, tag, (uint32_t) 0x00000002, buf);
	int sent = chan_config(sockfd);
	char buf[100];
	
	// while we still have a connection to coasters:	
	while (recv(sockfd, buf, 100, 0) > 0){
		printf("received: %d\n", recv(sockfd, buf, 100, 0));
	}
	if(sent < 0) exit(1);
}
