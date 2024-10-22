#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFSIZE 1<<17

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
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		return 1;
	}

	/*Convert portnum*/
	uint16_t port;
	if(convert_port_name(&port, argv[1]) != 0){
		fprintf(stderr, "Invalid port number: %s\n", strerror(errno));
		return 1;
	}

	/*Create socket*/
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		fprintf(stderr,"Error creating socket: %s\n", strerror(errno));
		return 1;
	}

    /*Bind socket*/
    sockaddr_in client;
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(port); 
    client.sin_addr.s_addr = htonl(INADDR_ANY); /* Bind to any address */

    if(bind(sockfd, (sockaddr *)&client, sizeof(client)) < 0) {
        fprintf(stderr, "Failed binding socket: %s\n", strerror(errno));
        if(close(sockfd)){
			fprintf(stderr,"Error closing socket: %s\n",strerror(errno));
			return 1;
		}
        return 1;
    }

	/*Send while receiving*/
	char buffer[BUFFSIZE];
	ssize_t received;
	sockaddr_in sender;
	socklen_t sender_len = sizeof(sender);

	while((received = recvfrom(sockfd, buffer, BUFFSIZE, 0, (sockaddr *)&sender, &sender_len)) >= 0){
		if (sendto(sockfd, buffer, received, 0, (sockaddr *)&sender, sender_len) < 0) {
			fprintf(stderr,"Error sending data: %s\n", strerror(errno));
			if(close(sockfd)<0){
				fprintf(stderr,"Error closing socket: %s\n",strerror(errno));
				return 1;
			}
			return 1;
		}
	}

	if(received < 0){
		fprintf(stderr,"Error receiving info: %s\n",strerror(errno));
		if(close(sockfd)<0){
			fprintf(stderr,"Error closing socket: %s\n",strerror(errno));
			return 1;
		}
		return 1;
	}

	if(close(sockfd)<0){
		fprintf(stderr,"Error closing file: %s\n",strerror(errno));
		return 1;
	}

	return 0;
}
