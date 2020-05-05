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
pthread_mutex_t mut_allt = PTHREAD_MUTEX_INITIALIZER; //Semaphore to stop all threads
pthread_cond_t con_allt = PTHREAD_COND_INITIALIZER; //Condition to broadcast all threads
pthread_cond_t con_server = PTHREAD_COND_INITIALIZER; //Condition to broadcast server
bool is_used = false;
int global_newfd = 0;

void key_listener() {
    int ch = 0;
    printf("Press q to kill all threads and terminate server\n");
    // Wait for character "q" to be pressed
    while (ch != 113) {
        ch = getchar();
    }
    kill(0, SIGKILL);
}

void signal_handler(int signal) {
    if (signal == SIGUSR1) {
        _exit(0);
    }
}

void *thread_request() {
    int newfd = 0;
    bool work = false;

    while (1) {
        work = false;

        pthread_mutex_lock(&mut_allt);
        if (is_used == true) {
            newfd = global_newfd;
            is_used = false;
            work = true;
            pthread_cond_broadcast(&con_server);
        } else {
            pthread_cond_wait(&con_allt, &mut_allt);
            if (is_used == true) {
                newfd = global_newfd;
                is_used = false;
                work = true;
                pthread_cond_broadcast(&con_server);
            }
        }
        pthread_mutex_unlock(&mut_allt);

        if (work == true) {
            if (newfd == -1) {
                perror("accept");
                continue;
            }
            handle_http_request(newfd);
        }
    }
}

int main(int argc, char **argv) {
    int newfd, n_threads;
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

    pthread_t all_tid[n_threads];

    for (int i = 0; i < n_threads; i++) {
        if (pthread_create(&all_tid[i], NULL, &thread_request, NULL) != 0) {
            printf("Error in thread creation!\n");
        }
    }

    int listenfd = get_listener_socket(port);
    if (listenfd < 0) {
        errExit("webserver: fatal error getting listening socket\n");
    }
    while (1) {
        socklen_t sin_size = sizeof their_addr;
        newfd = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);
        //--Warn threads of a new customer
        pthread_mutex_lock(&mut_allt);
        is_used = true;
        global_newfd = newfd;
        pthread_cond_broadcast(&con_allt);
        pthread_mutex_unlock(&mut_allt);
        //--Wait for the client to be taken by someone
        pthread_mutex_lock(&mut_allt);
        if (is_used == true) {
            pthread_cond_wait(&con_server, &mut_allt);
        }
        pthread_mutex_unlock(&mut_allt);
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
    }
}