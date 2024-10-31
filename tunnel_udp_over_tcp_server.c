#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*Simplification*/
#define BUFFSIZE (1 << 16)
#define MAX(a,b) ((a>b)?a:b)
#define PRINTERRMSG(msg) fprintf(stderr, "%s: %s\n", msg, strerror(errno))
#define ret return 1
#define retneg return -1
#define forever while(1)

typedef long long int muy_largo_t;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

/*Converts port name to a 16-bit unsigned integer*/
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

/*The good write*/
ssize_t better_write(int fd, const char *buf, size_t count) {
    size_t already_written = 0;
    ssize_t res_write;

    while (count > 0) {
        res_write = write(fd, buf + already_written, count);
        if (res_write <= 0) return res_write;
        already_written += res_write;
        count -= res_write;
    }
    return already_written;
}

/*Handles data forwarding between TCP and UDP sockets*/
static int run_server(int tcp_fd, int udp_fd) {
    char buffer[BUFFSIZE];
    ssize_t bytes_read;
    fd_set read_fds;

    forever {
        FD_ZERO(&read_fds);
        FD_SET(tcp_fd, &read_fds);
        FD_SET(udp_fd, &read_fds);
        int max_fd = MAX(tcp_fd, udp_fd) + 1;

        if (select(max_fd, &read_fds, NULL, NULL, NULL) < 0) {
            PRINTERRMSG("Error in select");
            return -1;
        }

        /* Forward data from TCP client to UDP server */
        if (FD_ISSET(tcp_fd, &read_fds)) {
            bytes_read = read(tcp_fd, buffer, BUFFSIZE);
            if (bytes_read < 0) {
                PRINTERRMSG("Error reading from TCP");
                return -1;
            }
            if (bytes_read == 0) {
                break; /* EOF */
            }

            if (send(udp_fd, buffer, bytes_read, 0) < 0) {
                PRINTERRMSG("Error sending to UDP");
                return -1;
            }
        }

        /* Forward data from UDP server to TCP client */
        if (FD_ISSET(udp_fd, &read_fds)) {
            bytes_read = recv(udp_fd, buffer, BUFFSIZE, 0);
            if (bytes_read < 0) {
                PRINTERRMSG("Error receiving from UDP");
                return -1;
            }
            if (bytes_read == 0) {
                break; /* EOF */
            }
            
            if (better_write(tcp_fd, buffer, bytes_read) < 0) {
                PRINTERRMSG("Error writing to TCP client");
                return -1;
            }
        }
    }
    return 0;
}


/*Main logic*/
int main(int argc, char *argv[]) {
    /*Argument check*/
    if (argc < 4) {
        PRINTERRMSG("Usage: tunnel_udp_over_tcp_server <tcp-port> <udp-server> <udp-port>");
        ret;
    }

    /*Setup*/
    const char *tcp_port_name = argv[1], *udp_server_ip = argv[2], *udp_port_name = argv[3];

    /*Convert port numbers*/
    uint16_t tcp_port;
    if (convert_port_name(&tcp_port, tcp_port_name) < 0) {
        PRINTERRMSG("Invalid TCP port number");
        ret;
    }



    /*TCP socket setup*/
    int tcp_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socketfd < 0) {
        PRINTERRMSG("Failed to create TCP socket");
        ret;
    }

    /*Binding setup*/
    sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(tcp_port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*Binding*/
    if (bind(tcp_socketfd, (sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        PRINTERRMSG("Failed to bind TCP socket");
        if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
        ret;
    }

    /*Get address info*/
    addrinfo hints,*res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
    if (getaddrinfo(udp_server_ip, udp_port_name, &hints, &res) != 0) {
        PRINTERRMSG("Error getting address info");
        if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
        ret;
    }

    /*Open UDP socket*/
    int udp_socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (udp_socketfd < 0) {
        PRINTERRMSG("Failed to create UDP socket");
        freeaddrinfo(res);
        if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
        ret;
    }

    /*Addressing*/
    addrinfo *current;
    for(current = res;current != NULL;current = current->ai_next) if(connect(udp_socketfd,current->ai_addr,current->ai_addrlen) == 0) break; 

    /*Address assignment failed*/
    if(current == NULL){
        PRINTERRMSG("Error assigning address to UDP port");
        freeaddrinfo(res);
        if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
        if(close(udp_socketfd) < 0) PRINTERRMSG("Error closing UDP socket");
        ret;
    }

    /*We don't need the address info anymore*/
    freeaddrinfo(res);

    /*Accept setup*/
    sockaddr_in client_address;
    socklen_t clientaddr_len = sizeof(client_address);
    int tcp_client_socketfd;

    /*Listen on TCP*/
    if (listen(tcp_socketfd, 1) < 0) {
        PRINTERRMSG("Failed to listen on TCP socket");
        if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
        if(close(udp_socketfd) < 0) PRINTERRMSG("Error closing UDP socket");
        ret;
    }

    /*Wait for connections*/
    if((tcp_client_socketfd = accept(tcp_socketfd,(sockaddr *) &client_address, &clientaddr_len)) < 0) {
        PRINTERRMSG("Failed to accept connection");
        if(close(tcp_socketfd) < 0) PRINTERRMSG("Error closing TCP socket");
        if(close(udp_socketfd) < 0) PRINTERRMSG("Error closing UDP socket");
        ret;
    }

    /*Run the server loop to handle data transfer*/
    int result = run_server(tcp_client_socketfd, udp_socketfd);

    /*Cleanup*/
    if(close(tcp_client_socketfd) < 0){
        PRINTERRMSG("Error closing TCP client socket");
        ret;
    }
    if(close(tcp_socketfd) < 0){
        PRINTERRMSG("Error closing TCP socket");
        ret;
    }
    if(close(udp_socketfd) < 0){
        PRINTERRMSG("Error closing UDP socket");
        ret;
    }

    return result < 0 ? 1 : 0;
}