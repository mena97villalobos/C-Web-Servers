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
#include <stdbool.h>
#include <signal.h>

const char *help_string = "Usage: server <puerto> <max-threads>\n";

//Variables to synchronize all threads
pthread_mutex_t new_req; //Semaphore to tell threads new request
pthread_mutex_t req_consumed; //Semaphore to tell main thread to continue
int listenfd;
int global_newfd = 0;

void initSemaphore(pthread_mutex_t *semaphore) {
    if (pthread_mutex_init(semaphore, NULL) != 0) {
        printf("Semaphore initialization failed\n");
        exit(1);
    }
}

void *thread_request() {
    int newfd = 0;
    while (1) {
        pthread_mutex_lock(&new_req);
        newfd = global_newfd;
        pthread_mutex_unlock(&req_consumed);
        if (server_stopped()) {
            printf("Stopping thread");
            break;
        }

        handle_http_request(newfd);
        shutdown(newfd, SHUT_RDWR);
        if (server_stopped()) {
            shutdown(listenfd, SHUT_RDWR);
        }
    }
    return NULL;
}

void key_listener() {
    int ch = 0;
    printf("Press q to kill all processes and terminate server\n");
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

int main(int argc, char **argv) {
    int n_threads;
    char port[6] = "";
    struct sockaddr_storage their_addr;

    signal(SIGUSR1, signal_handler);

    // Launch a fork to handle stdin and check if user wants to stop the program
    pid_t keyboard_listener_pid = fork();
    if (keyboard_listener_pid != 0) {
        key_listener();
        return 0;
    }

    //Check the count of arguments
    if (argc < 3)
        errExit(help_string);

    // Validate program arguments
    if (!validate_port(argv[1])) errExit("Invalid port");
    if (!validate_number(argv[2])) errExit("Invalid n-threads");

    // Clean variables
    memset(port, 0, 6);

    // Copy data to variables
    strncpy(port, argv[1], strlen(argv[1]));
    n_threads = atoi(argv[2]);


    initSemaphore(&new_req);
    initSemaphore(&req_consumed);
    pthread_mutex_lock(&new_req);
    pthread_mutex_lock(&req_consumed);

    pthread_t all_tid[n_threads];

    for (int i = 0; i < n_threads; i++) {
        if (pthread_create(&all_tid[i], NULL, &thread_request, NULL) != 0) {
            printf("Error in thread creation!\n");
        }
    }

    listenfd = get_listener_socket(port);
    if (listenfd < 0) {
        fprintf(stderr, "Webserver: fatal error getting listening socket\n");
        exit(1);
    }
    while (1) {
        socklen_t sin_size = sizeof their_addr;
        global_newfd = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);
        if (global_newfd == -1) {
            if (server_stopped()) {
                for (int i = 0; i < n_threads; i++) {
                    pthread_mutex_unlock(&new_req);
                    pthread_mutex_lock(&req_consumed);
                }
                break;
            } else {
                perror("Accept Invalid.");
                continue;
            }
        }

        //--Warn threads of a new customer
        pthread_mutex_unlock(&new_req);
        pthread_mutex_lock(&req_consumed);
    }

    for (int i = 0; i < n_threads; i++) { /* Wait until all threads are finished */
        pthread_join(all_tid[i], NULL);
    }
}