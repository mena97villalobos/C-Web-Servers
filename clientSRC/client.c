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

#define OUT_FILE "salida.txt"

const char *help_string = "Usage: client <maquina> <puerto> <archivo> <n-thread> <n-ciclos>\n";

char *codificar(char string[]);

char *concat(const char *s1, const char *s2);


void errExit(const char *str) {
    fprintf(stderr, str);
    exit(-1);
}

int main(int argc, char **argv) {
    struct addrinfo *result, hints;
    int srvfd, rwerr = 42;
    char *request, buf[5000000], port[6], ip[16], filename[15];

    if (argc < 6)
        errExit(help_string);

    // Validate program arguments
    if (!validate_ip(argv[1])) errExit("Invalid IP Address");
    if (!validate_port(argv[2])) errExit("Invalid port");
    if (!validate_number(argv[4])) errExit("Invalid n-threads");
    if (!validate_number(argv[5])) errExit("Invalid n-cycles");

    memset(port, 0, 6);
    memset(ip, 0, 16);

    strncpy(ip, argv[1], strlen(argv[1]));
    strncpy(port, argv[2], strlen(argv[2]));
    strncpy(filename, argv[3], strlen(argv[3]));

    printf(filename);
    printf("\n");
    printf(ip);
    printf("\n");
    fflush(stdout);

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (0 != getaddrinfo(ip, port, &hints, &result))
        errExit("getaddrinfo");

    // Create socket after retrieving the inet protocol to use (getaddrinfo)
    srvfd = socket(result->ai_family, SOCK_STREAM, 0);

    if (srvfd < 0)
        errExit("socket()");

    if (connect(srvfd, result->ai_addr, result->ai_addrlen) == -1) {
        errExit("connect");
    }
    write(srvfd, filename, strlen(filename));

    // Turn down socket
    shutdown(srvfd, SHUT_WR);

    FILE *destFile = fopen(OUT_FILE, "wb");

    int c = 0;
    while (rwerr > 0) {
        rwerr = read(srvfd, buf, 5000000);
        printf(c++);
        fwrite(buf, 1, rwerr, destFile);
    }
    fclose(destFile);
    close(srvfd);

    return 0;

}
