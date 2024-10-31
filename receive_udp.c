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

#define BUFFSIZE (1<<16)
#define PRINTERRMSG(msg) fprintf(stderr, "%s: %s\n", msg, strerror(errno))
#define ret return 1

typedef long long int muy_largo_t; // Easier to write
typedef struct sockaddr_in sockaddr_in; 
typedef struct sockaddr sockaddr;

/*Better write*/
ssize_t better_write(int fd, const char *buf, size_t count) {
    size_t already_written = 0,to_be_written = count;
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

int main(int argc, char *argv[]) {
    /*Check if we have port as arg*/
    if (argc < 2) {
        PRINTERRMSG("Usage: ./receive_udp <port>");
        ret;
    }

    /*Get port num*/
    uint16_t port;
    if (convert_port_name(&port, argv[1]) < 0) {
        PRINTERRMSG("Error converting port number");
        ret;
    }

    /*Create socket*/
    int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    /*Socket failed*/
    if (socket_fd < 0) {
        PRINTERRMSG("Error creating socket");
        ret;
    }

    /*Bind socket*/
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = 0; 

    /*Binding*/
    if(bind(socket_fd, (sockaddr *)&server_address, sizeof(server_address)) < 0) {
        PRINTERRMSG("Error binding socket");
        if(close(socket_fd)<0) PRINTERRMSG("Error closing socket");
        ret;
    }

    /*Receive data*/
    char buffer[BUFFSIZE];
    int bytes_received;

    /*Receive and print data*/
    while ((bytes_received = recv(socket_fd, buffer, BUFFSIZE, 0)) > 0) {
        if (better_write(STDOUT_FILENO, buffer, bytes_received) < 0) {
            PRINTERRMSG("Error writing to stdout");
            if(close(socket_fd)) PRINTERRMSG("Error closing socket");
            ret;
        }
    }

    /*FAILED RECV*/
    if (bytes_received < 0) {
        PRINTERRMSG("Error receiving data");
        if(close(socket_fd)){
            PRINTERRMSG("Error closing socket");
            ret;
        };
        return 1;
    }

    /*Close socket*/
    if (close(socket_fd) < 0) {
        PRINTERRMSG("Error closing socket");
        ret;
    }

    /*All clear*/
    return 0;
}
