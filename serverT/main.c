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
#include <signal.h>

const char *help_string = "Usage: server <puerto>\n";

void key_listener() {
    int ch = 0;
    printf("Press q to kill all threads and terminate server\n");
    // Wait for character "q" to be pressed
    while (ch != 113) {
        ch = getchar();
    }
    kill(0, SIGUSR1);
}

void signal_handler(int signal) {
    if (signal == SIGUSR1) {
        _exit(0);
    }
}

void *thread_request(void *arguments) {
    int newfd = *((int *) arguments);
    if (newfd == -1) {
        perror("accept");
    } else {
        handle_http_request(newfd);
    }
    free(arguments);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int newfd;
    char port[6] = "";
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];

    signal(SIGUSR1, signal_handler);

    // Launch a fork to handle stdin and check if user wants to stop the program
    pid_t keyboard_listener_pid = fork();
    if (keyboard_listener_pid != 0) {
        key_listener();
        return 0;
    }

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
        errExit("webserver: fatal error getting listening socket\n");
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
                close(newfd);
                continue;
            }
            *arg_thread = newfd;
            //---
            int err = pthread_create(&var_thread, NULL, &thread_request, arg_thread);

            if (err != 0) {
                printf("Error in thread creation!, error: %s\n", strerror(err));
                free(arg_thread);
                close(newfd);
                continue;
            }
            err = pthread_detach(var_thread);
            if (err != 0) {
                printf("Error in thread detach!, error: %s\n", strerror(err));
            }
            pthread_detach(var_thread);
            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        }
    }
}
