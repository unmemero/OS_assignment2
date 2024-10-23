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
	/*Check if we have server address and port*/
	if(argc < 3){
		fprintf(stderr, "Usage: ./send_udp <domain> <port> %s\n", strerror(errno));
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

	/*from socket.h*/
	/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, -1 for errors.

   This function is a cancellation point and therefore not marked with*/

	/*"Connect"*/
    addrinfo *current_element;
    for(current_element = result; current_element != NULL; current_element = current_element->ai_next){
        if(connect(sockfd, current_element->ai_addr, current_element->ai_addrlen) == 0){
            break; 
        }
    }

    /*Connection failed*/
    if(current_element == NULL){
        fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
        freeaddrinfo(result);
		if(close(sockfd) < 0){
			fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
			return 1;
		}
        return 1;
    }

	/*Send message*/
	ssize_t read_bytes;
	while((read_bytes = read(STDIN_FILENO, buffer, BUFFSIZE)) > 0){
		if(send(sockfd,buffer,read_bytes,0) < 0){
			fprintf(stderr, "Error sending: %s\n", strerror(errno));
			freeaddrinfo(result);
			if(close(sockfd) < 0){
				fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
				return 1;
			}
			return 1;
		}
	}

	/*Read failed*/
	if(read_bytes < 0){
		fprintf(stderr, "Error reading from STDIN: %s\n", strerror(errno));
		freeaddrinfo(result);
		if(close(sockfd) < 0){
			fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
			return 1;
		}
		return 1;
	}

	/*Send empty packet*/
	if(send(sockfd, NULL, 0, 0) < 0){
		fprintf(stderr, "Error sending data: %s\n", strerror(errno));
		freeaddrinfo(result);
		if(close(sockfd) < 0){
			fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
			return 1;
		}
		return 1;
	}

	/*Closing aand cleaning*/
	freeaddrinfo(result);
	if(close(sockfd) < 0){
		fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}