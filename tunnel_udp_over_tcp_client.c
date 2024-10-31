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

/*Buffer sizes*/
#define UDPBUFFSIZE (1<<16)
#define TCPBUFFSIZE (1<<16) + 2
#define RECONSTRUCTSIZE (1<<17) + 4
/*Function simplidications*/
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define PRINTERRMSG(msg) fprintf(stderr, "%s: %s\n", msg, strerror(errno))
#define ret return 1
/*Sorry, I was trying to be funny*/
#define forever while(1)
#define MYEOF 0

/*Typedefs for code clarity*/
typedef long long int muy_largo_t;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

/*Better write function cause write is not good on its own*/
ssize_t better_write(int fd, const char *buf, size_t count) {
    size_t already_written = 0, to_be_written = count;
    ssize_t res_write;
    if (count == 0) return count;

    while (to_be_written > 0) {
        res_write = write(fd, &buf[already_written], to_be_written);
        if (res_write < 0) return res_write; 
        if (res_write == 0) return already_written;
        already_written += res_write;
        to_be_written -= res_write;
    }
    return already_written;
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

/*Memcpy implementation, cause we can't use level 3 function calls*/
void my_memcpy(void *dest, const void *orig, size_t length){
	char *dest_char = (char *)dest;
	const char *orig_char = (const char *)orig;
	for(size_t i = 0;i<length;i++) dest_char[i] = orig_char[i];
}

/*Main implementation of tunneling for client*/
int main(int argc, char **argv){
	if(argc != 4){
		PRINTERRMSG("Usage <udp_port> <tcp_server_ip> <tcp_port>");
		ret;
	}

	char * udp_port_name = argv[1], *tcp_server_ip = argv[2], *tcp_port_name = argv[3];

	/*Get UDP port number*/
	uint16_t udp_port;
	if(convert_port_name(&udp_port, udp_port_name) < 0){
		PRINTERRMSG("Unable to convert UDP port number");
		ret;
	}

	/*Create UDP socket*/
	int udp_socketfd = socket(AF_INET, SOCK_DGRAM,0);
	if(udp_socketfd < 0){
		PRINTERRMSG("Failed creating UDP sockeT");
		ret;
	}

	/*Bind UDP socket*/
	sockaddr_in udp_server_address;
	memset(&udp_server_address,0,sizeof(udp_server_address));
	udp_server_address.sin_family = AF_INET;
	udp_server_address.sin_port = htons(udp_port);
	udp_server_address.sin_addr.s_addr = 0;

	/*Binding*/
	if(bind(udp_socketfd,(sockaddr *)&udp_server_address,sizeof(udp_server_address)) < 0){
		PRINTERRMSG("Error binding UDP socket");
		if(close(udp_socketfd) < 0) PRINTERRMSG("Error closing UDP socket");
		ret;
	}

	/*Open TCP connection*/
	addrinfo hints, *result;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	int gai_code = getaddrinfo(tcp_server_ip, tcp_port_name, &hints, &result);
	if(gai_code != 0){
		PRINTERRMSG("Error getting address info");
		if(close(udp_socketfd) < 0) PRINTERRMSG("Error closing UDP socket");
		ret;
	}

	/*Create TCP socket*/
	int tcp_socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(tcp_socketfd < 0){
		PRINTERRMSG("Error creating TCP socket");
		freeaddrinfo(result);
		if(close(udp_socketfd) < 0) PRINTERRMSG("Error closing UDP socket");
		ret;
	}

	/*Connect TCP socket*/
	addrinfo *current_element;
	for(current_element = result; current_element != NULL; current_element = current_element->ai_next){
		if(connect(tcp_socketfd, current_element->ai_addr,current_element->ai_addrlen) == 0){
			break;
		}
	}

	/*Connection failed*/
	if(current_element == NULL){
		PRINTERRMSG("Error connecting to server");
		freeaddrinfo(result);
		if(close(udp_socketfd) < 0) {
			PRINTERRMSG("Error closing UDP socket");
			if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
			ret;
		}
		if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
		ret;
	}

	/*We don't need this, so goodbye address info, you were a good friend*/
	freeaddrinfo(result);

	/*We will use select to check for activity on both sockets*/
	fd_set read_fds;
	int maxfd = MAX(udp_socketfd, tcp_socketfd) + 1;
	char udp_buffer[UDPBUFFSIZE], tcp_buffer[TCPBUFFSIZE], reconstruction_buffer[RECONSTRUCTSIZE];
	int udp_available = 0;
	sockaddr_in udp_sender_address;
	socklen_t udp_sender_address_length = sizeof(udp_sender_address);
	size_t reconstruction_index = 0, expected_length = 0;
	ssize_t bytes_received;
	uint16_t network_length;

	/*Until break condition*/
	forever{
		/*Set FD set*/
		FD_ZERO(&read_fds);
		FD_SET(udp_socketfd, &read_fds);
		FD_SET(tcp_socketfd, &read_fds);
		
		int select_result = select(maxfd, &read_fds,NULL,NULL,NULL);
		if(select_result < 0){
			PRINTERRMSG("Error in select");
			if(close(udp_socketfd) < 0) {
				PRINTERRMSG("Error closing UDP socket");
				if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
				ret;
			}
			if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
			ret;
		}

		/*UDP socket has data*/
		if(FD_ISSET(udp_socketfd,&read_fds)){
			/*Receive from UDP socket and store sende address*/
			if((bytes_received = recvfrom(udp_socketfd,udp_buffer,UDPBUFFSIZE,0,(sockaddr *)&udp_sender_address,&udp_sender_address_length)) < 0){
				PRINTERRMSG("Error receiving data");
				ret;
			}

			/*Mark UDP sender available*/
			udp_available = 1;

			/*Message length*/
			uint16_t message_size = MIN(bytes_received,UDPBUFFSIZE);
			char message_buffer[UDPBUFFSIZE + 2];
			network_length = htons((uint16_t)message_size);
			my_memcpy(message_buffer, &network_length, 2);
			if(bytes_received > 0) my_memcpy(message_buffer+2,udp_buffer,message_size);

			/*Write into TCP FD*/
			size_t required_write_bytes = bytes_received + 2;
			if(bytes_received == 0){
				required_write_bytes = 2;
			}
			if(better_write(tcp_socketfd, message_buffer, required_write_bytes) < 0){
				PRINTERRMSG("Error writing to TCP socket");
				if(close(udp_socketfd) < 0){
					PRINTERRMSG("Error closing UDP socket");
					if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
					ret;
				}
				ret;
			}
		}

		/*TCP socket has data*/
		if(FD_ISSET(tcp_socketfd,&read_fds)){
			/*Read from TCP socket*/
			if((bytes_received = read(tcp_socketfd,tcp_buffer,TCPBUFFSIZE)) < 0){
				PRINTERRMSG("Error reading from TCP socket");
				if(close(udp_socketfd) < 0) {
					PRINTERRMSG("Error closing UDP socket");
					if(close(tcp_socketfd)) PRINTERRMSG("Error closing TCP socket");
					ret;
				}
				if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
				ret;
			}
			
			/*EOF reached*/
			if(bytes_received == MYEOF) break;

			/*Process and reconstruct received message*/
			for(ssize_t i = 0;i<bytes_received;i++){
				/*Reconstruction index out of bounds*/
				if(reconstruction_index >= RECONSTRUCTSIZE){
					PRINTERRMSG("Reconstruction index is greater than reconstruction buffer size");
					if(close(udp_socketfd) < 0){
						PRINTERRMSG("Error closing UDP socket");
						if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
						ret;
					}
					if(close(tcp_socketfd) < 0) PRINTERRMSG("Erro closing TCP socket");
					ret;
				}

				reconstruction_buffer[reconstruction_index] = tcp_buffer[i];
				reconstruction_index++;

				/*First 2 bytes are header*/
				if(reconstruction_index == 2 && expected_length == 0){
					my_memcpy(&network_length,reconstruction_buffer,2);
					expected_length = ntohs(network_length);
					if(expected_length > UDPBUFFSIZE){
						PRINTERRMSG("Invalid message length");
						if(close(udp_socketfd) < 0){
							PRINTERRMSG("Error closing UDP socket");
							if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
							ret;
						}
						if(close(tcp_socketfd) < 0) PRINTERRMSG("Erro closing TCP socket");
						ret;
					}
				}

				/*Complete message*/
				if(expected_length > 0 && reconstruction_index == expected_length + 2){
					/*SendTo message to UDP port if UDP packet received*/
					if(udp_available){
						if(sendto(udp_socketfd,reconstruction_buffer+2,expected_length,0,(sockaddr *)&udp_sender_address, udp_sender_address_length) < 0){
							PRINTERRMSG("Error sending to UDP socket");
							if(close(udp_socketfd) < 0){
								PRINTERRMSG("Error closing UDP socket");
								if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
								ret;
							}
							if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
							ret;
						}
					}

					/*Reset variables*/
					reconstruction_index = 0;
					expected_length = 0;
				}

				/*Reset variables if no message received*/
				if(expected_length == 0 && reconstruction_index > 2) reconstruction_index = 0;
			}
		}
	}

	/*Close sockets*/
	if(close(udp_socketfd) < 0){
		PRINTERRMSG("Error closing UDP socket");
		if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
		ret;
	}
	if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");

	return 0;
}