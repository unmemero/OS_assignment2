#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>

#define SENDBUFFSIZE 480
#define RECVBUFFSIZE 65536
typedef struct addrinfo addrinfo;

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

int main(int argc, char *argv[]){
	/*Check args*/
	if(argc < 3){
		fprintf(stderr, "Usage: %s <domain> <port>\n", argv[0]);
		return 1;
	}

	/*Keep track of buffer, destination, and port*/	
	char *dest_addr = argv[1], *port = argv[2], send_buffer[SENDBUFFSIZE], recv_buffer[RECVBUFFSIZE];

	/*Get address info*/
	addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;
	int gai_code = getaddrinfo(dest_addr, port, &hints, &result);
	if(gai_code != 0){
		fprintf(stderr, "Error getting address info: %s\n", gai_strerror(gai_code));
		return 1;
	}

	/*Create socket*/
	int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(sockfd < 0){
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
		freeaddrinfo(result);
		return 1;
	}

	/*Check*/
    addrinfo *current_element;
    for(current_element = result; current_element != NULL; current_element = current_element->ai_next){
		/*If successful connection, exit*/
        if(connect(sockfd, current_element->ai_addr, current_element->ai_addrlen) == 0){
            break; 
        }
    }

    /*Connection failed*/
    if(current_element == NULL){
        fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
        freeaddrinfo(result);
		if(close(sockfd) < 0){
			fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
			return 1;
		}
        return 1;
    }

	/*Go while write*/
	ssize_t bytes_read, bytes_sent, bytes_received;
	fd_set read_fds;
	int max_fd = (STDIN_FILENO > sockfd) ? STDIN_FILENO : sockfd, select_result;
	while(1){
		/*Clear FD set*/
		FD_ZERO(&read_fds);

		/*Add FDs to set*/
		FD_SET(STDIN_FILENO, &read_fds);
		FD_SET(sockfd, &read_fds);

		/*Select*/
		select_result = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
		if(select_result < 0){
			fprintf(stderr, "Error selecting: %s\n", strerror(errno));
			freeaddrinfo(result);
			if(close(sockfd) < 0){
				fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
				return 1;
			}
			break;
		}

		/*Check STDIN for data*/
		if(FD_ISSET(STDIN_FILENO, &read_fds)){

			/*Read*/
			bytes_read = read(STDIN_FILENO, send_buffer, SENDBUFFSIZE);
			/*Reed failed*/
			if(bytes_read < 0){
				fprintf(stderr, "Error reading from STDIN: %s\n", strerror(errno));
				freeaddrinfo(result);
				if(close(sockfd) < 0){
					fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
					return 1;
				}
				return 1;
			}

			if(bytes_read == 0){
				/*Send empty*/
				if((bytes_sent = send(sockfd,send_buffer,bytes_read,0))< 0){
					fprintf(stderr, "Error sending data: %s\n", strerror(errno));
					freeaddrinfo(result);
					if(close(sockfd) < 0){
						fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
						return 1;
					}
					return 1;
				}
				break;
			}

			/*Send full*/
			if((bytes_sent = send(sockfd,send_buffer,bytes_read,0))< 0){
				fprintf(stderr, "Error sending data: %s\n", strerror(errno));
				freeaddrinfo(result);
				if(close(sockfd) < 0){
					fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
					return 1;
				}
				return 1;
			}
		}

		/*Check you sock for gifts*/
		if(FD_ISSET(sockfd, &read_fds)){
			/*Receive*/
			bytes_received = recv(sockfd, recv_buffer, RECVBUFFSIZE, 0);
			/*Receive failed*/
			if(bytes_received < 0){
				fprintf(stderr, "Error receiving data: %s\n", strerror(errno));
				freeaddrinfo(result);
				if(close(sockfd) < 0){
					fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
					return 1;
				}
				return 1;
			}

			/*Empty buffer received*/
			if(bytes_received == 0){
				break;
			}

			/*Write*/
			if((bytes_sent  = better_write(STDOUT_FILENO, recv_buffer, bytes_received)) < 0){
				fprintf(stderr, "Error writing to STDOUT: %s\n", strerror(errno));
				freeaddrinfo(result);
				if(close(sockfd) < 0){
					fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
					return 1;
				}
				return 1;
			}
		}

	}

	/*Clean up*/
	freeaddrinfo(result);
	if(close(sockfd) < 0){
		fprintf(stderr, "Error closing socket: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
