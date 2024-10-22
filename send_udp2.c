#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFSIZE 480

int main(int argc, char *argv[]){
	/*Check if we have server address and port*/
	if(argc < 3){
		fprintf(stderr, "Usage: ./send_udp <domain> <port> %s\n", strerror(errno));
		return 1;
	}

	/*Initialize buffer, get server name and port*/
	char buffer[BUFFSIZE], *server_name = argv[1], *port = argv[2];
	
	struct addrinfo hints, *result;
	/*Initialize struct with 0's*/
	memset(&hints, 0, sizeof(hints));
	/*Prep hints*/
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	result = NULL;

	/*Get address info*/
	int gai_code = getaddrinfo(server_name, port, &hints, &result);
	if(gai_code != 0){
		fprintf(stderr, "Error getting address info: %s\n", gai_strerror(gai_code));
		return 1;
	}

	/*Create socket (IPv4, Datagram for UDP, IP PROTOCOL)*/
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	/*Check if socket was created*/
	if(socket_fd < 0){
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
		return 1;
	}

	/*Bind*/
	if(bind(socket_fd, result->ai_addr, result->ai_addrlen) < 0){
		fprintf(stderr, "Error binding socket: %s\n", strerror(errno));
		freeaddrinfo(result);
		close(socket_fd);
		return 1;
	}

	/*Send message*/
	ssize_t bytes_read;
	while((bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE)) > 0){
		if(send(socket_fd, buffer, bytes_read, 0) < 0){
			fprintf(stderr, "Error sending message: %s\n", strerror(errno));
			freeaddrinfo(result);
			close(socket_fd);
			return 1;
		}
	}

	/*Read terminated successfully*/
	if(bytes_read == 0){
		if(send(socket_fd,NULL,0,0)){
			fprintf(stderr, "Error sending message: %s\n", strerror(errno));
			freeaddrinfo(result);
			close(socket_fd);
			return 1;
		}
	}

	/*Check if there was an error reading from STDIN*/
	if(bytes_read < 0){
		fprintf(stderr, "Error reading from STDIN: %s\n", strerror(errno));
		freeaddrinfo(result);
		close(socket_fd);
		return 1;
	}

	/*Free address info and close socket*/
	freeaddrinfo(result);
	if(close(socket_fd)){
		fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}