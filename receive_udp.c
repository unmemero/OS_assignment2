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

#define BUFFSIZE 480

typedef long long int muy_largo_t; // Easier to write

/*Better write*/
ssize_t better_write(int fd, const char *buf, size_t count) {
    size_t already_written = 0;
    size_t to_be_written = count;
    ssize_t res_write;

    if (count == 0) return count;

    while (to_be_written > 0) {
        res_write = write(fd, &buf[already_written], to_be_written);
        if (res_write < 0) {
            return res_write; // Error
        }
        if (res_write == 0) {
            return already_written; // Nothing written, stop
        }
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
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    /*Get port num*/
    uint16_t port;
    if (convert_port_name(&port, argv[1]) < 0) {
        fprintf(stderr, "Invalid port number: %s\n", argv[1]);
        return 1;
    }

    /*Create socket*/
    int socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    /*Socket failed*/
    if (socket_fd < 0) {
        fprintf(stderr, "Failed creating socket: %s\n", strerror(errno));
        return 1;
    }

    /*Bind socket*/
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = 0; 

    /*Binding*/
    if(bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Failed binding socket: %s\n", strerror(errno));
        close(socket_fd);
        return 1;
    }

    /*Receive data*/
    char buffer[BUFFSIZE];
    int bytes_received;

    /*Receive and print data*/
    while ((bytes_received = recv(socket_fd, buffer, BUFFSIZE, 0)) > 0) {
        if (bytes_received == 0) {
            printf("Received empty packet. Shutting down server...\n");
            break;
        }
        
        if (better_write(STDOUT_FILENO, buffer, bytes_received) < 0) {
            fprintf(stderr, "Failed writing data: %s\n", strerror(errno));
            close(socket_fd);
            return 1;
        }
    }

    /*FAILED RECV*/
    if (bytes_received < 0) {
        fprintf(stderr, "Failed receiving data: %s\n", strerror(errno));
        close(socket_fd);
        return 1;
    }

    /*Close socket*/
    if (close(socket_fd) < 0) {
        fprintf(stderr, "Failed closing socket: %s\n", strerror(errno));
        return 1;
    }

    /*All clear*/
    return 0;
}
