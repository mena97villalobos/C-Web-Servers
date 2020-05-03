#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "argValidator.h"
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define OUT_FILE "/dev/null"
#define BUFFER_SIZE 1024


const char *help_string = "Usage: detener <ip> <port>\n";

//Test variables
double *mgby_sec;
double *first_request_sec;
int error_request = 0;
int clientCounter = 0;


//Function definitions
int final_header(const char string[], int maxCheck);

void substring(const char [], char[], int, int);

void errExit(const char *str);

void make_request(char *request, char *port, char *ip);


//C final header function implementation, return -1 if the standard is incorrect
int final_header(const char string[], int maxCheck) {
    int i = 0, j = 0, flat = 0;

    while (i < maxCheck) {
        if (string[i] == '\n') {
            if (j != 0 && j + 1 == i) {
                i += 1;
                flat = 1;
                break;
            }
            j = i;
        }
        i++;
    }
    if (flat)return i;
    return -1;
}

// C error exit function implementation
void errExit(const char *str) {
    fprintf(stderr, "%s", str);
    exit(-1);
}

// C substring function implementation
void substring(const char s[], char sub[], int p, int l) {
    int c = 0;
    while (c < l) {
        sub[c] = s[p + c - 1];
        c++;
    }
    sub[c] = '\0';
}

// Code to create a request
void make_request(char *request, char *ip, char *port) {
    //Define initial variables
    struct addrinfo *result, hints;
    int srvfd;
    char temp_buf[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    unsigned long total_bytes = 0;

    memset(temp_buf, 0, BUFFER_SIZE);
    memset(buf, 0, BUFFER_SIZE);

    // Clean variables
    memset(&hints, 0, sizeof(struct addrinfo));

    // Define internet protocol
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (0 != getaddrinfo(ip, port, &hints, &result)) {
        fprintf(stderr, "%s\n", "getaddrinfo");
        return;
    }

    // Create socket after retrieving the inet protocol to use (getaddrinfo)
    srvfd = socket(result->ai_family, SOCK_STREAM, 0);

    if (srvfd < 0) {
        fprintf(stderr, "%s\n", "socket()");
        error_request++;
        return;
    }

    if (connect(srvfd, result->ai_addr, result->ai_addrlen) == -1) {
        fprintf(stderr, "%s\n", "connect");
        error_request++;
        return;
    }


    //Send http request
    write(srvfd, request, strlen(request));

    // Turn down socket
    shutdown(srvfd, SHUT_WR);

    // Open file destination exit
    FILE *destFile = fopen(OUT_FILE, "wb");

    // Initial time
    //Check a remove the initial header
    int bytesReceived = recv(srvfd, temp_buf, BUFFER_SIZE, 0);
    total_bytes += bytesReceived;
    if (bytesReceived < 0) {
        fprintf(stderr, "%s\n", "recv() failed");
        error_request++;
        return;
    }
    int header_final_location = final_header(temp_buf, bytesReceived);
    if (header_final_location == -1) {
        fprintf(stderr, "%s\n", "Error http format in the server");
        return;
    }
    substring(temp_buf, buf, header_final_location + 1, bytesReceived - header_final_location + 1);
    bytesReceived -= header_final_location;

    //Cycling to get all files
    while (bytesReceived != 0) {
        fwrite(buf, bytesReceived, 1, destFile);
        bytesReceived = recv(srvfd, buf, BUFFER_SIZE, 0);
        total_bytes += bytesReceived;
        if (bytesReceived < 0) {
            fprintf(stderr, "%s\n", "recv() failed");
            error_request++;
            return;
        }
    }
    //Close files and free memory
    fclose(destFile);
    close(srvfd);
}


int main(int argc, char **argv) {

    //Check the count of arguments
    if (argc != 3)
        errExit(help_string);

    
    char *ip = argv[1];
    
    char *port = argv[2];
    // Validate program arguments
    if (!validate_ip(ip)) errExit("Invalid IP Address");
    if (!validate_port(port)) errExit("Invalid port");

    //Create http request
    char request[98 + strlen(ip)];
    sprintf(request, "GET /DETENER?PK=12345 HTTP/1.1\nHost: %s\nUser-agent: simple-http client\n\n", ip);


    printf("Trying to stop server at %s:%s",ip,port);
    make_request(request,ip,port);
    return 0;
}
