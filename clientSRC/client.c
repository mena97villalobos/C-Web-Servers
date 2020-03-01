
# include <stdio.h>
# include <stdlib.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netdb.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
#include "argValidator.h"
#include <libexplain/connect.h>

#define OUT_FILE "~/Desktop/salida.txt"

const char *help_string = "Usage: client <maquina> <puerto> <archivo> <n-thread> <n-ciclos>\n";

char *codificar(char string[]);

char *concat(const char *s1, const char *s2);


void errExit(const char *str) {
    fprintf(stderr, str);
    exit(-1);
}

int main(int argc, char *argv[]) {
    struct addrinfo *result, hints;
    int srvfd, rwerr;
    char *request, buf[5000000];

    if (argc < 6)
        errExit(help_string);

    // Validate program arguments
    if (!validate_ip(argv[1])) {
        errExit("Invalid IP Address");
    }
    if (!validate_port(argv[2])) {
        errExit("Invalid port");
    }
    if (!validate_number(argv[4])) {
        errExit("Invalid n-threads");
    }
    if (!validate_number(argv[5])) {
        errExit("Invalid n-cycles");
    }

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (0 != getaddrinfo(argv[1], argv[2], &hints, &result))
        errExit("getaddrinfo");

    // Create socket after retrieving the inet protocol to use (getaddrinfo)
    srvfd = socket(result->ai_family, SOCK_STREAM, 0);

    if (srvfd < 0)
        errExit("socket()");

    if (connect(srvfd, result->ai_addr, result->ai_addrlen) == -1) {
        fprintf(stderr, "%s\n", explain_connect(srvfd, result->ai_addr, result->ai_addrlen));
        errExit("connect");
    }

    char *serverFileName = codificar(argv[3]);
    write(srvfd, serverFileName, strlen(serverFileName));

    // Turn down socket
    shutdown(srvfd, SHUT_WR);

    FILE *destFile = fopen(OUT_FILE, "w");

    do {
        rwerr = read(srvfd, buf, 5000000);
        write(fileno(destFile), buf, rwerr);
    } while (rwerr > 0);

    close(srvfd);

    return 0;

}

char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *codificar(char string[]) {
    FILE *fp;
    char *result = (char *) malloc(1035 * sizeof(char));
    char *c1 = concat("echo \"", string);
    char *command = concat(c1,
                           "\" | sed 's/ /%20/g;s/!/%21/g;s/\"/%22/g;s/#/%23/g;s/\\\\$/%24/g;s/\\\\&/%26/g;s/'\\\''/%27/g;s/(/%28/g;s/)/%29/g;s/:/%%3A/g'");

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL) {
        while (fp == NULL) {
            fp = popen(command, "r");
        }
    }

    fgets(result, 1034, fp);
    pclose(fp);
    return result;
}
