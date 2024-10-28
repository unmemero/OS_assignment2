#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef long long int muy_largo_t;

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



int main(int argc, char *argv[]){
	/*Argument check*/
	if(argc < 4){
		fprintf(stderr, "Usage: %s <server_ip> <server_port> <local_port>\n", argv[0]);
		return 1;
	}

	/*Port conversion*/
	uint16_t udp_port, tcp_port;
	if(convert_port_name(&udp_port, argv[2]) != 0){
		fprintf(stderr, "Invalid port number: %s\n", strerror(errno));
		return 1;
	}

	if(convert_port_name(&tcp_port, argv[3]) != 0){
		fprintf(stderr, "Invalid port number: %s\n", strerror(errno));
		return 1;
	}

	/*Create udp socket*/
	int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sockfd < 0){
		fprintf(stderr,"Error creating socket: %s\n", strerror(errno));
		return 1;
	}

	
	return 0;
}
