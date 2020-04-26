#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../headers/net.h"

#define BACKLOG 100

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int get_listener_socket(char *port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {

        // Try to make a socket based on this candidate interface
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        // SO_REUSEADDR prevents the "address already in use" errors
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            close(sockfd);
            freeaddrinfo(servinfo);
            return -2;
        }

        // Bind this socket to this local IP address. This
        // associates the file descriptor (the socket descriptor) that
        // we will read and write on with a specific IP address.
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }
    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "webserver: failed to find local address\n");
        return -3;
    }

    // Start listening. This is what allows remote computers to connect
    // to this socket/IP.
    if (listen(sockfd, BACKLOG) == -1) {
        close(sockfd);
        return -4;
    }

    return sockfd;
}