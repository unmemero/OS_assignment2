#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFSIZE 480
#define PRINTERRMSG(msg) fprintf(stderr, "%s: %s\n", msg, strerror(errno))
#define ret return 1

typedef struct addrinfo addrinfo;

int main(int argc, char *argv[]){
	/*Check if we have server address and port*/
	if(argc < 3){
		PRINTERRMSG("Usage <destination_address> <port>");
		ret;
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
	/*GAI code failed*/
	if(gai_code != 0){
		PRINTERRMSG("Error getting address info");
		ret;
	}

	/*Create socket*/
	int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	/*Failed creating socket*/
	if(sockfd < 0){
		PRINTERRMSG("Error creating socket");
		freeaddrinfo(result);
		ret;
	}

	/*from socket.h*/
	/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, -1 for errors.

   This function is a cancellation point and therefore not marked with*/

	/*"Connect" by iterating through each element from the linked list until connect is successfull or end of list is null*/
    /*If successful connection, exit*/
	addrinfo *current_element;
    for(current_element = result; current_element != NULL; current_element = current_element->ai_next) if(connect(sockfd, current_element->ai_addr, current_element->ai_addrlen) == 0) break; 

    /*Connection failed*/
    if(current_element == NULL){
		PRINTERRMSG("Error connecting to server");
		freeaddrinfo(result);
		if(close(sockfd) < 0) PRINTERRMSG("Error closing socket");
        ret;
    }

	/*
		"We don't need this, so goodbye address info, and you were a good friend"
			- Alec Guinness
	*/
	freeaddrinfo(result);

	/*Read until empry end message*/
	ssize_t read_bytes;
	while((read_bytes = read(STDIN_FILENO, buffer, BUFFSIZE)) > 0){
		if(send(sockfd,buffer,read_bytes,0) < 0){
			PRINTERRMSG("Error sending data");
			if(close(sockfd) < 0) PRINTERRMSG("Error closing socket");
			ret;
		}
	}

	/*Read failed*/
	if(read_bytes < 0){
		PRINTERRMSG("Error reading data");
		if(close(sockfd) < 0) PRINTERRMSG("Error closing socket");
		ret;
	}

	/*Send empty packet*/
	if(send(sockfd, NULL, 0, 0) < 0){
		PRINTERRMSG("Error sending data");
		if(close(sockfd) < 0) PRINTERRMSG("Error closing socket");
		ret;
	}

	/*Closing aand cleaning*/
	if(close(sockfd) < 0){
		PRINTERRMSG("Error closing socket");
		ret;
	}

	return 0;
}