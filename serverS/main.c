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

const char *help_string = "Usage: server <puerto>\n";

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
        fprintf(stderr, "webServer: fatal error getting listening socket\n");
        exit(1);
    }
    while (1) {
        socklen_t sin_size = sizeof their_addr;
        newfd = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);
        if (newfd == -1) {
            perror("Accept Invalid");
            continue;
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
        handle_http_request(newfd);
    }
}
