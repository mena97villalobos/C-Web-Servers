#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../headers/commonHttpFunctions.h"
#include "../headers/net.h"
#include "../headers/argValidator.h"
#include <pthread.h>

const char *help_string = "Usage: server <puerto>\n";

void *thread_request(void *arguments) {
    int newfd = *((int *) arguments);
    if (newfd == -1) {
        perror("accept");
    } else {
        handle_http_request(newfd);
        close(newfd);
    }
    free(arguments);
    return NULL;
}

int main(int argc, char **argv) {
    int newfd;
    char port[6] = "";
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];

    //Check the count of arguments
    if (argc < 2)
        errExit(help_string);

    // Validate program arguments
    if (!validate_port(argv[1])) errExit("Invalid port");

    // Clean variables
    memset(port, 0, 6);

    // Copy data to variables
    strncpy(port, argv[1], strlen(argv[1]));

    int listenfd = get_listener_socket(port);
    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }
    while (1) {
        socklen_t sin_size = sizeof their_addr;
        pthread_t var_thread;
        newfd = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);

        if (newfd == -1) {
            fprintf(stderr, "accept error");
        } else {

            //--Arguments to send to thread
            int *arg_thread = malloc(sizeof(*arg_thread));
            if (arg_thread == NULL) {
                fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
                exit(EXIT_FAILURE);
            }
            *arg_thread = newfd;
            //---

            if (pthread_create(&var_thread, NULL, &thread_request, arg_thread) != 0) {
                printf("Error in thread creation!\n");
            }
            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        }
    }
}
