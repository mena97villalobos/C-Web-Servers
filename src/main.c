#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include <openssl/md5.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>


#define PORT "8080"


void main() {

    int newfd;
    int newfdmodify;
    struct sockaddr_storage their_addr;
    struct sockaddr_storage addr_modify;

    int listenfd = get_listener_socket(PORT);

    socklen_t sin_size = sizeof their_addr;

    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    char *buffer[100];
    while (1) {
        int a = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);
        read(a, buffer, 100);
        printf(buffer);
    }
}