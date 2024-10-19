#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT 8080
#define BUFFSIZE 480

int main(int argc, char *argv[]){
	/*Check if we have server address and port*/
	if(argc < 3){
		fprintf(stderr,"Usage: ./send_udp <domain> <port> %s\n", strerror(errno));
		return 1;
	}

	/*Initialize buffer, get server	memset(&hints, 0, sizeof(hints)); name (a little bit easier to remember with a var name), and port no.*/
	char buffer[BUFFSIZE], *server_name=argv[1], *port=argv[2];
	/*Create socket(IPv4, Datagram for UDP,IP PROTOCOL)*/
	int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	/*Check if socket was created*/
	if(socket_fd < 0){
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
		return 1;
	}
	
	/*Get hints about address with getaddrinfo*/
	struct addrinfo hints, *result;
	/*Initialize struct with 0's*/
	memset(&hints, 0, sizeof(hints));
	/*Prep hints*/
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = 0;
	result = NULL;

	/*Get the address info*/
	int getaddrinfo_code = getaddrinfo(server_name, port, &hints, &result);
	/*Augh, I, the gaicode failed and died*/
	if(getaddrinfo_code != 0){
		fprintf(stderr, "Error getting address info: %s\n", gai_strerror(getaddrinfo_code));
		return 1;
	}

	/*Send until STDIN is empty*/
	size_t bytes_read;
	while((bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE))>0){
		/*Send to used in UDP to send data through sockets. Uses The fd of the socket, the buffer, 
		number of bytes to send, The flags (no special handling so 0), the destination address, and the size of the address*/
		if(sendto(socket_fd,buffer,bytes_read,0,result->ai_addr,result->ai_addrlen) < 0){
			fprintf(stderr, "Error sending data: %s\n", strerror(errno));
			freeaddrinfo(result);
			/*Closing failed*/
			if(close(socket_fd) < 0){
				fprintf(stderr, "Error closing file: %s\n", strerror(errno));
				return 1;
			}
			return 1;
		}
	}

	/*Send empty response*/
	if(sendto(socket_fd,NULL,0,0,result->ai_addr,result->ai_addrlen) < 0){
		fprintf(stderr, "Error sending data: %s\n", strerror(errno));
		freeaddrinfo(result);
		if(close(socket_fd) < 0){
			fprintf(stderr, "Error closing file: %s\n", strerror(errno));
			return 1;
		}
		return 1;
	}

	/*Cleanup and return*/
	freeaddrinfo(result);
	if(close(socket_fd) < 0){
		fprintf(stderr, "Error closing file: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
