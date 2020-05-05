#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "../headers/commonHttpFunctions.h"
#include "../headers/net.h"
#include "../headers/argValidator.h"
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

const char *help_string = "Usage: server <port> <num_forks>\n";

void *create_shared_memory(size_t);

pthread_condattr_t get_conditional_attribute();

pthread_mutexattr_t get_mutex_attributes();

void *create_shared_memory(size_t size) {
    int protection = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED | MAP_ANONYMOUS | MAP_SYNC;

    return mmap(NULL, size, protection, flags, -1, 0);
}

void *child_process(
        pthread_mutex_t *mut_allt,
        pthread_cond_t *con_allt,
        pthread_cond_t *con_server,
        bool *is_used,
        const int socket_fd
) {
    int newfd = 0;
    bool work = false;
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof their_addr;

    while (1) {
        work = false;

        pthread_mutex_lock(mut_allt);
        if (*is_used == true) {
            newfd = accept(socket_fd, (struct sockaddr *) &their_addr, &sin_size);
            if (newfd == -1) {
                perror("accept");
                continue;
            }
            memset(is_used, false, sizeof(bool));
            work = true;
            pthread_cond_broadcast(con_server);
        } else {
            pthread_cond_wait(con_allt, mut_allt);
            if (*is_used == true) {
                newfd = accept(socket_fd, (struct sockaddr *) &their_addr, &sin_size);
                if (newfd == -1) {
                    perror("accept");
                    continue;
                }
                memset(is_used, false, sizeof(bool));
                work = true;
                pthread_cond_broadcast(con_server);
            }
        }
        pthread_mutex_unlock(mut_allt);

        if (work == true) {
            handle_http_request(newfd);
            shutdown(newfd, SHUT_RDWR);
            close(newfd);
        }
    }
    return NULL;
}

pthread_condattr_t get_conditional_attribute() {
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    return cattr;
}

pthread_mutexattr_t get_mutex_attributes() {
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    return mattr;
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
    ////Variables to synchronize all processes
    pthread_mutex_t *mut_allt;
    pthread_cond_t *con_allt;
    pthread_cond_t *con_server;

    bool *is_used;
    int *global_newfd;

    int newfd, n_threads;
    char port[6] = "";
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    pid_t pid;

    signal(SIGUSR1, signal_handler);

    // Launch a fork to handle stdin and check if user wants to stop the program
    pid_t keyboard_listener_pid = fork();
    if (keyboard_listener_pid != 0) {
        key_listener();
        return 0;
    }

    pthread_mutexattr_t mattr = get_mutex_attributes();
    pthread_condattr_t cattr = get_conditional_attribute();

    mut_allt = (pthread_mutex_t *) create_shared_memory(sizeof(pthread_mutex_t));
    pthread_mutex_init(mut_allt, &mattr);
    con_allt = (pthread_cond_t *) create_shared_memory(sizeof(pthread_cond_t));
    pthread_cond_init(con_allt, &cattr);
    con_server = (pthread_cond_t *) create_shared_memory(sizeof(pthread_cond_t));
    pthread_cond_init(con_server, &cattr);

    is_used = create_shared_memory(sizeof(bool));
    global_newfd = create_shared_memory(sizeof(int));

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

    int listenfd = get_listener_socket(port);
    if (listenfd < 0) {
        errExit("webserver: fatal error getting listening socket\n");
    }

    for (int i = 0; i < n_threads; i++) {
        pid = fork();
        if (pid == 0) {
            child_process(mut_allt, con_allt, con_server, is_used, listenfd);
            return 0;
        }
    }

    fd_set set;
    int rv;
    FD_ZERO(&set); /* clear the set */
    FD_SET(listenfd, &set); /* add our file descriptor to the set */

    while (1) {

        rv = select(listenfd + 1, &set, NULL, NULL, NULL);

        if (rv == -1) {
            perror("select"); /* an error accured */
            return 1;
        } else {
            //--Warn threads of a new customer
            pthread_mutex_lock(mut_allt);
            memset(is_used, true, sizeof(bool));
            memcpy(global_newfd, &newfd, sizeof(int));
            pthread_cond_broadcast(con_allt);
            pthread_mutex_unlock(mut_allt);

            //--Wait for the client to be taken by someone
            pthread_mutex_lock(mut_allt);
            if (*is_used == true) {
                pthread_cond_wait(con_server, mut_allt);
            }
            pthread_mutex_unlock(mut_allt);
            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        }
    }
    return 0;
}
