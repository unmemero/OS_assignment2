#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFSIZE 480
typedef struct addrinfo addrinfo;


int main(int argc, char *argv[]){
	/*Check args*/
	if(argc < 3){
		fprintf(stderr, "Usage: %s <domain> <port>\n", argv[0]);
		return 1;
	}

	/*Keep track of buffer, destination, and port*/	
	char *dest_addr = argv[1], *port = argv[2], buffer[BUFFSIZE];

	/*Get address info*/
	addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	int gai_code = getaddrinfo(dest_addr, port, &hints, &result);
	if(gai_code != 0){
		fprintf(stderr, "Error getting address info: %s\n", gai_strerror(gai_code));
		return 1;
	}

	/*Create socket*/
	int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(sockfd < 0){
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
		freeaddrinfo(result);
		return 1;
	}

	/*Send message*/
	ssize_t bytes_read;
	while((bytes_read = read(STDIN_FILENO,buffer,BUFFSIZE)) > 0){
		ssize_t bytes_sent = sendto(sockfd, buffer, bytes_read, 0, result->ai_addr, result->ai_addrlen);
		
	}

	if(bytes_read < 0){
		fprintf(stderr, "Error readig from STTDIN: %s\n", strerror(errno));
		freeaddrinfo(result);
		close(sockfd);
		return 1;
	}


	return 0;
}
