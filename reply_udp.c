#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFSIZE (1<<16)
#define PRINTERRMSG(msg) fprintf(stderr, "%s: %s\n", msg, strerror(errno))
#define ret return 1
#define forever while(1)

typedef long long int muy_largo_t;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

/*Returns port value in pointer, and status code as regular*/
static int convert_port_name(uint16_t *port, const char *port_name) {
    if (port_name == NULL || *port_name == '\0') return -1;

    char *end;
    muy_largo_t nn = strtoll(port_name, &end, 0);
    if (*end != '\0') return -1;
    if (nn < 0) return -1;

    uint16_t t = (uint16_t)nn;
    muy_largo_t tt = (muy_largo_t)t;
    if (tt != nn) return -1;

    *port = t;
    return 0;
}

int main(int argc, char *argv[]){
	/*Check arguments >1*/
	if(argc < 2){
		PRINTERRMSG("Usage: ./reply_udp <port>");
		ret;
	}

	/*Convert portnum*/
	uint16_t port;
	if(convert_port_name(&port, argv[1]) != 0){
		PRINTERRMSG("Error converting port number");
		ret;
	}

	/*Create socket*/
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		PRINTERRMSG("Error creating socket");
		ret;
	}

    /*Bind socket*/
    sockaddr_in client;
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(port); 
    client.sin_addr.s_addr = htonl(INADDR_ANY); 
	
	/* Bind to any address */
    if(bind(sockfd, (sockaddr *)&client, sizeof(client)) < 0) {
		PRINTERRMSG("Error binding socket");
        if(close(sockfd)) PRINTERRMSG("Error closing socket");		
        ret;
    }

	/*Send while receiving*/
	char buffer[BUFFSIZE];
	ssize_t received;
	sockaddr_in sender;
	socklen_t sender_len = sizeof(sender);
	forever{
		/*Receive data*/
		received = recvfrom(sockfd, buffer, BUFFSIZE, 0, (sockaddr *)&sender, &sender_len);
		if(received >= 0){
			/*Send data*/
			if (sendto(sockfd, buffer, received, 0, (sockaddr *)&sender, sender_len) < 0) {
				PRINTERRMSG("Error sending data");
				if(close(sockfd)<0) PRINTERRMSG("Error closing socket");
				ret;
			}
		}

		/*Receive failed*/
		if(received < 0){
			PRINTERRMSG("Error receiving data");
			if(close(sockfd)<0) PRINTERRMSG("Error closing socket");
			ret;
		}
	}

	/*Close socket*/
	if(close(sockfd)<0){
		PRINTERRMSG("Error closing socket");
		ret;
	}

	return 0;
}