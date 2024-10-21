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
	/*Create socket (IPv4, Datagram for UDP, IP PROTOCOL)*/
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
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	result = NULL;

	/*Get the address info*/
	int getaddrinfo_code = getaddrinfo(server_name, port, &hints, &result);
	if(getaddrinfo_code != 0){
		fprintf(stderr, "Error getting address info: %s\n", gai_strerror(getaddrinfo_code));
		return 1;
	}

	/*Bitconeeeect*/
	if(connect(socket_fd, result->ai_addr, result->ai_addrlen) < 0){
		fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
		freeaddrinfo(result);
		close(socket_fd);
		return 1;
	}

	/*Send until STDIN is empty*/
	size_t bytes_read;
	while((bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE)) > 0){
		/* Use sendto to send data to the server address */
		if(send(socket_fd, buffer, bytes_read, 0) < 0){
			fprintf(stderr, "Error sending data: %s\n", strerror(errno));
			freeaddrinfo(result);
			close(socket_fd);
			return 1;
		}
	}

	/*Send empty response*/
	if(send(socket_fd, NULL, 0, 0) < 0){
		fprintf(stderr, "Error sending empty packet: %s\n", strerror(errno));
		freeaddrinfo(result);
		close(socket_fd);
		return 1;
	}

	/*Cleanup and return*/
	freeaddrinfo(result);
	if(close(socket_fd) < 0){
		fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}




/*
	QUESTIONS FOR DR. LAUTER
	1. Do we handle null terminator on client or server side? (Tell him you think on server until EOF)
	2. Send() on its own doesn't work, need to use sendto() or establish connection with connect. Mention UDP is connectionless.
	(base) pills@rpc assignment2 main $ ./send_udp 129.108.156.68 8080
	hello
	Error sending data: Destination address required
	Expected results only working with connect()
*/
