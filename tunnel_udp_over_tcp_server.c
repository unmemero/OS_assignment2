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

#define BUFFSIZE 65536

typedef long long int muy_largo_t;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

/* Converts port name to a 16-bit unsigned integer */
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

/* Improved write function to handle partial writes */
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

/* Handles data forwarding between TCP and UDP sockets */
static int run_server(int tcp_fd, int udp_fd) {
    char buffer[BUFFSIZE];
    ssize_t bytes_read;
    fd_set read_fds;

    for (;;) {
        FD_ZERO(&read_fds);
        FD_SET(tcp_fd, &read_fds);
        FD_SET(udp_fd, &read_fds);
        int max_fd = (tcp_fd > udp_fd) ? tcp_fd : udp_fd;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select failed");
            return -1;
        }

        // Forward data from TCP client to UDP server
        if (FD_ISSET(tcp_fd, &read_fds)) {
            bytes_read = read(tcp_fd, buffer, BUFFSIZE);
            if (bytes_read < 0) {
                perror("Error reading from TCP");
                return -1;
            }
            if (bytes_read == 0) break;

            if (send(udp_fd, buffer, bytes_read, 0) < 0) {
                perror("Error sending to UDP");
                return -1;
            }
        }

        // Forward data from UDP server to TCP client
        if (FD_ISSET(udp_fd, &read_fds)) {
            bytes_read = recv(udp_fd, buffer, BUFFSIZE, 0);
            if (bytes_read < 0) {
                perror("Error receiving from UDP");
                return -1;
            }
            if (bytes_read == 0) break;

            if (better_write(tcp_fd, buffer, bytes_read) < 0) {
                perror("Error writing to TCP");
                return -1;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <TCP port> <UDP server> <UDP port>\n", argv[0]);
        return 1;
    }

    // Convert port numbers
    uint16_t tcp_port, udp_port;
    if (convert_port_name(&tcp_port, argv[1]) < 0) {
        fprintf(stderr, "Invalid TCP port number: %s\n", argv[1]);
        return 1;
    }
    if (convert_port_name(&udp_port, argv[3]) < 0) {
        fprintf(stderr, "Invalid UDP port number: %s\n", argv[3]);
        return 1;
    }

    const char *udp_server = argv[2];

    // TCP socket setup
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) {
        perror("Failed to create TCP socket");
        return 1;
    }

    sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(tcp_port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(tcp_sock, (sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("Failed to bind TCP socket");
        close(tcp_sock);
        return 1;
    }

    if (listen(tcp_sock, 1) < 0) {
        perror("Failed to listen on TCP socket");
        close(tcp_sock);
        return 1;
    }

    // UDP socket setup without hints
    addrinfo *res;
    if (getaddrinfo(udp_server, argv[3], NULL, &res) != 0) {
        perror("Failed to get address info for UDP server");
        close(tcp_sock);
        return 1;
    }
    int udp_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (udp_sock < 0) {
        perror("Failed to create UDP socket");
        freeaddrinfo(res);
        close(tcp_sock);
        return 1;
    }
    freeaddrinfo(res);

    // Accept incoming TCP connection
    sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int tcp_client_sock = accept(tcp_sock, (sockaddr *)&clientaddr, &clientaddr_len);
    if (tcp_client_sock < 0) {
        perror("Failed to accept TCP connection");
        close(tcp_sock);
        close(udp_sock);
        return 1;
    }

    // Run the server loop to handle data transfer
    int result = run_server(tcp_client_sock, udp_sock);

    // Cleanup
    close(tcp_client_sock);
    close(tcp_sock);
    close(udp_sock);

    return result < 0 ? 1 : 0;
}