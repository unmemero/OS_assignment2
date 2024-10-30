#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define DGRAMSIZE 65536

typedef long long int muy_largo_t;
typedef struct addrinfo addrinfo;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

/*Better write*/
ssize_t better_write(int fd, const char *buf, size_t count) {
    size_t already_written = 0;
    size_t to_be_written = count;
    ssize_t res_write;

    if (count == 0) return count;

    while (to_be_written > 0) {
        res_write = write(fd, &buf[already_written], to_be_written);
        if (res_write < 0) {
            return res_write; 
        }
        if (res_write == 0) {
            return already_written;
        }
        already_written += res_write;
        to_be_written -= res_write;
    }
    return already_written;
}

/*Cleanup*/
int cleanup(int udp_sockfd, int tcp_sockfd, addrinfo *result){
	/*Free address info if not null*/
	if(result != NULL) freeaddrinfo(result);

	/*Check if udp and tcp are set*/
	int udp_set = (udp_sockfd > -1) ? 1 : 0;
	int tcp_set = (tcp_sockfd > -1) ? 1 : 0;

	/*Close udp if set*/
	if(udp_set){

		/*Failed closing*/
		if(close(udp_sockfd) < 0){
			fprintf(stderr, "Error closing socket: %s\n", strerror(errno));

			/*Close TCP if set*/
			if(tcp_set){

				/*Failed TCP*/
				if(close(tcp_sockfd) < 0){
					fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
				}
			}
			return 1;
		}
	}
	/*UDP was successfull in closing, now close TCP*/
	if(tcp_set){
		if(close(tcp_sockfd) < 0){
			fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
			return 1;
		}
	}

	return 0;
}

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

/**/
int main(int argc, char *argv[]){
	int cleanup_code;

	/*Argument check*/
	if(argc < 4){
		fprintf(stderr, "Usage: %s <tcp_port> <server_ip> <udp_port>\n", argv[0]);
		return 1;
	}

	/*Setup*/
	char *server_ip = argv[2];

	/*Port conversion*/
	uint16_t udp_port, tcp_port;
	if(convert_port_name(&udp_port, argv[3]) != 0){
		fprintf(stderr, "Invalid port number: %s\n", strerror(errno));
		return 1;
	}
	if(convert_port_name(&tcp_port, argv[1]) != 0){
		fprintf(stderr, "Invalid port number: %s\n", strerror(errno));
		return 1;
	}

	/*Open and bind UDP socket*/
	int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sockfd < 0){
		fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
		return 1;
	}
	
	sockaddr_in udp_address;
	memset(&udp_address, 0, sizeof(udp_address));
	udp_address.sin_family = AF_INET;
	udp_address.sin_port = htons(udp_port);
	udp_address.sin_addr.s_addr = INADDR_ANY;

	if(bind(udp_sockfd, (sockaddr *)&udp_address, sizeof(udp_address)) < 0){
		cleanup_code = cleanup(udp_sockfd, -1, NULL);
		return 1;
	}

	/*Connect through TCP*/
	addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	int gai_code = getaddrinfo(argv[1], argv[2], &hints, &result);

	if(gai_code != 0){
		cleanup_code = cleanup(udp_sockfd, -1, NULL);
		return 1;
	}
	int tcp_sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(tcp_sockfd < 0){
		 cleanup_code = cleanup(udp_sockfd, -1, result);
		return 1;
	}

	addrinfo *current;
	for(current = result; current != NULL; current = current->ai_next){
		if(connect(tcp_sockfd, current->ai_addr, current->ai_addrlen) == 0){
			break;
		}
	}

	if(current == NULL){
		cleanup_code = cleanup(udp_sockfd, tcp_sockfd, result);
		return 1;
	}

	/*Loop setup*/
	fd_set read_fds;
	int max_fd = (udp_sockfd > tcp_sockfd) ? udp_sockfd : tcp_sockfd, select_result;
	uint16_t udp_packet_size_network;
	ssize_t udp_dgram_size;
	char udp_buffer[DGRAMSIZE], tcp_message[DGRAMSIZE + 2];
	sockaddr_in sender;
	socklen_t sender_len = sizeof(sender);
	
	/*SPEEN 2 WEEN*/
	while(1){
		/*Clear FD's*/
		FD_ZERO(&read_fds);

		/*Add FD's*/
		FD_SET(udp_sockfd, &read_fds);
		FD_SET(tcp_sockfd, &read_fds);

		/*Select*/
		select_result = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
		if(select_result < 0){
			cleanup_code = cleanup(udp_sockfd, tcp_sockfd, result);
			return 1;
		}

		/*Check UDP for data*/
		if(FD_ISSET(udp_sockfd, &read_fds)){

			/*Read from UDP port*/
			if((udp_dgram_size = recvfrom(udp_sockfd,udp_buffer,DGRAMSIZE,0,(sockaddr *) &sender,&sender_len))<0){
				cleanup_code = cleanup(udp_sockfd, tcp_sockfd, result);
				return 1;
			}
			
			/*Format packet with 2 bytes of length*/
			udp_packet_size_network = htons((uint16_t)udp_dgram_size);
			memcpy(tcp_message, &udp_packet_size_network,2);
			if(udp_dgram_size > 0){
				memcpy(tcp_message + 2, udp_buffer, udp_dgram_size);
			}

			/*Write to tcp socket fd*/
			if(better_write(tcp_sockfd,tcp_message,udp_dgram_size + 2) < 0){
				cleanup_code = cleanup(udp_sockfd, tcp_sockfd, result);
				return 1;
			}
			printf("Sent %ld bytes to TCP\n", udp_dgram_size); //FOR DEBUGGING		
		}

		/*Check TCP for data*/
		if(FD_ISSET(tcp_sockfd, &read_fds)){

		}
	}

	return cleanup(udp_sockfd, tcp_sockfd, result);
}
