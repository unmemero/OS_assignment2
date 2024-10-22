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

	/*Create hints*/
	struct addrinfo hints, *result;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	/*Get address info*/
	int gaicode = getaddrinfo(server_name,port,&hints,&result);
	if(gaicode != 0){
		fprintf(stderr,"Error getting address info\n");
		return 1;
	}

	/*Iterate through GAI result LL*/
	struct addrinfo *i;
	int sockfd;

	for(i=result;i != NULL;i=i->ai_next){
		sockfd = socket(i->ai_family,i->ai_socktype,i->ai_protocol);
		if(sockfd < 0){
			continue;
		}
		break;
	}

	if(i == NULL){
		fprintf(stderr,"Error finding socket");
		freeaddrinfo(result);
		return 1;
	}

	ssize_t num_bytes_read;
	while ((num_bytes_read = read(STDIN_FILENO, buffer, BUFFSIZE)) > 0) {
		if (send(sockfd, buffer, num_bytes_read, 0) == -1) {
			fprintf(stderr,"Error sending\n");
			close(sockfd);
			freeaddrinfo(result);
			return 1;
		}
	}

    if (num_bytes_read == -1) {
        perror("read");
        close(sockfd);
        freeaddrinfo(result);
        return 1;
    }

    // Send an empty packet to indicate EOF
    if (send(sockfd, NULL, 0, 0) == -1) {
        perror("send EOF");
        close(sockfd);
        freeaddrinfo(result);
        return 1;
    }

    // Clean up
    close(sockfd);
    freeaddrinfo(result);

	return 0;
}