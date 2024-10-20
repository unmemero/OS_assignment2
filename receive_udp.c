#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef long long int muy_largo_t; // Easier to write

/*Returns port value in pointer, and status code as regulae*/
static int convert_port_name(uint16_t *port, const char *port_name){
	/*Port name doesn't exist, or null char*/
	if(port_name == NULL || *port_name == '\0')return -1;

	/*Converts string to long long int*/
	char *end;
	muy_largo_t nn = strtoll(port_name, &end, 0);
	if(*end != '\0')return -1;

	/*Checks if port is in range*/
	if(nn < (muy_largo_t)0)return -1;

	muy_largo_t t = (uint16_t)nn;
	muy_largo_t tt = (muy_largo_t)t;

	/*Checks if port is in range*/
	if(tt != nn)return -1;

	/*Returns port value*/
	*port = (uint16_t)nn;

	/*All clear*/
	return 0;
}


int main(int argc, char *argv[]){
	/*Check if we have port as arg*/
	if(argc < 2){
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		return 1;
	}
	
	/*Get port num*/
	uint16_t port;
	if(convert_port_name(&port, argv[1]) < 0){
		fprintf(stderr, "Invalid port number: %s\n", argv[1]);
		return 1;
	}

	/*Create socket*/
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/*Socket failed*/
	if(sockfd < 0){
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
