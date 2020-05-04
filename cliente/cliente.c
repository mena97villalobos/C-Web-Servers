#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "argValidator.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define OUT_FILE "/dev/null"
#define BUFFER_SIZE 1024

//Bar
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

//Colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char *help_string = "Usage: client <ip> <port> <file_name> <n-thread> <n-cycles> [v]\n";

//Test variables
int *total_success;
long *total_bytes;
long *total_ms;
long *first_request_msec;

int totalThreads = 0;
int executedThreads = 0;

int error_request = 0;
int clientCounter = 0;
sem_t mutex;

//Struct for thread arguments

volatile int verbose = 0;

struct arg_thread {
    char *request;
    char *port;
    char *ip;
    int n_cycles;
    int id_thread;
};

//Function definitions
int final_header(const char string[], int maxCheck);

void errExit(const char *str);

void make_request(char *request, char *port, char *ip, int thread_id, int id_cycle);

void *thread_request(void *arguments);

void printProgress(double percentage);

double calculate_transfered_MB(int n_threads);

int calculate_successes(int n_threads);

//C function to print de progress

void printProgress(double percentage) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

long tick(void) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

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

// Code to create a request

void make_request(char *request, char *port, char *ip, int thread_id, int id_cycle) {
    //Define initial variables
    struct addrinfo *result, hints;
    int srvfd;
    char buf[BUFFER_SIZE];
    unsigned long total_request_bytes;

    memset(buf, 0, BUFFER_SIZE);

    // Clean variables
    memset(&hints, 0, sizeof(struct addrinfo));

    // Define internet protocol
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (0 != getaddrinfo(ip, port, &hints, &result)) {
        if (verbose) {
            fprintf(stderr, "%s\n", "getaddrinfo");
        }
        return;
    }

    // Create socket after retrieving the inet protocol to use (getaddrinfo)
    srvfd = socket(result->ai_family, SOCK_STREAM, 0);

    if (srvfd < 0) {
        if (verbose) {
            fprintf(stderr, "%s\n", "Error socket()\n");
        }
        sem_wait(&mutex);
        error_request++;
        sem_post(&mutex);
        return;
    }

    if (connect(srvfd, result->ai_addr, result->ai_addrlen) == -1) {
        if (verbose) {
            fprintf(stderr, "%s\n", "Error on connect\n");
        }
        sem_wait(&mutex);
        error_request++;
        sem_post(&mutex);
        return;
    }

    //Send http request
    write(srvfd, request, strlen(request));

    // Turn down socket
    shutdown(srvfd, SHUT_WR);

    // Open file destination exit
    FILE *destFile = fopen(OUT_FILE, "wb");

    // Initial time
    clock_t begin = tick();
    //Check a remove the initial header
    int bytesReceived = recv(srvfd, buf, BUFFER_SIZE, 0);
    clock_t end_first = tick();
    if (bytesReceived < 0) {
        if (verbose) {
            fprintf(stderr, "%s\n", "recv() failed");
        }
        sem_wait(&mutex);
        error_request++;
        sem_post(&mutex);
        return;
    }
    int header_final_location = final_header(buf, bytesReceived);
    total_request_bytes = header_final_location;
    if (header_final_location == -1) {
        if (verbose) {
            fprintf(stderr, "%s\n", "Error http format in the server");
        }
        sem_wait(&mutex);
        error_request++;
        sem_post(&mutex);
        return;
    }
    buf[header_final_location + 1] = '\0';
    char *contentLengthPointer = strstr(buf, "Content-Length: ");
    if (contentLengthPointer == 0) {
        if (verbose) {
            fprintf(stderr, "%s\n", "Content-Lenght not found.");
        }
        sem_wait(&mutex);
        error_request++;
        sem_post(&mutex);
        return;
    }
    int contentLength;
    sscanf(&(contentLengthPointer[16]), "%d", &contentLength);
    total_request_bytes += contentLength;
    //Already read some bytes
    contentLength -= (bytesReceived - header_final_location);

    //Cycling to get all files
    while (contentLength != 0) {
        bytesReceived = recv(srvfd, buf, BUFFER_SIZE, 0);
        if (bytesReceived < 0) {
            if (verbose) {
                fprintf(stderr, "%s\n", "recv() failed");
            }
            sem_wait(&mutex);
            error_request++;
            sem_post(&mutex);
            return;
        }
        contentLength -= bytesReceived;
    }

    clock_t end = tick();

    //No mutex required as every thread uses its own mem
    total_bytes[thread_id] += total_request_bytes;
    first_request_msec[thread_id] += (end_first - begin);
    total_ms[thread_id] += (end - begin);
    ++total_success[thread_id];

    //Close files and free memory
    fclose(destFile);
    close(srvfd);


}

void *thread_request(void *arguments) {
    struct arg_thread args = *((struct arg_thread *) arguments);
    for (int i = 0; i < args.n_cycles; ++i) {
        sem_wait(&mutex);
        clientCounter += 1;
        sem_post(&mutex);
        if (verbose) {
            printf("Total requests: %i (Thread Id: %d, Exec %d).\n", clientCounter, args.id_thread, i);
        }
        make_request(args.request, args.port, args.ip, args.id_thread, i);

        sem_wait(&mutex);
        executedThreads += 1;
        printProgress((double) executedThreads / (double) totalThreads);
        sem_post(&mutex);
    }

    free(arguments);
    return NULL;
}

double calculate_transfered_MB(int n_threads) {
    double total_MB = 0;
    for (int i = 0; i < n_threads; ++i) {
        total_MB += total_bytes[i];
    }
    // Conver megabytes to Megabytes
    return total_MB / 1000000.0;
}

/**
 * Calculates total ms
 * @param n_threads
 * @return 
 */
double calculate_total_msec(int n_threads) {
    double total_sec = 0;
    for (int i = 0; i < n_threads; ++i) {
        total_sec += total_ms[i];
    }
    return total_sec;
}

double calculate_total_acceptance_msec(int n_threads) {
    double request_msec = 0;
    for (int i = 0; i < n_threads; ++i) {
        request_msec += first_request_msec[i];
    }
    return request_msec;
}

int calculate_successes(int n_threads) {
    int total_succ = 0;
    for (int i = 0; i < n_threads; ++i) {
        total_succ += total_success[i];
    }
    return total_succ;
}

int main(int argc, char **argv) {
    //Define initial variables
    int n_threads, n_cycles;
    char port[6] = "", ip[16] = "", filename[1000] = "";
    sem_init(&mutex, 0, 1);

    //Check the count of arguments
    if (argc < 6 || argc > 7)
        errExit(help_string);

    // Validate program arguments
    if (!validate_ip(argv[1])) errExit("Invalid IP Address");
    if (!validate_port(argv[2])) errExit("Invalid port");
    if (!validate_number(argv[4])) errExit("Invalid n-threads");
    if (!validate_number(argv[5])) errExit("Invalid n-cycles");

    if (argc == 7) {
        if (strcmp(argv[6], "v") == 0) {
            printf("Verbose enabled\n");
            verbose = 1;
        } else {
            errExit(help_string);
        }
    }
    // Clean variables
    memset(port, 0, 6);
    memset(ip, 0, 16);
    memset(filename, 0, 1000);

    // Copy data to variables
    strncpy(ip, argv[1], strlen(argv[1]));
    strncpy(port, argv[2], strlen(argv[2]));
    strncpy(filename, argv[3], strlen(argv[3]));
    n_threads = atoi(argv[4]);
    n_cycles = atoi(argv[5]);

    //Define global variables
    total_bytes = (long *) calloc(n_threads, sizeof(long));
    total_ms = (long *) calloc(n_threads, sizeof(long));
    first_request_msec = (long *) calloc(n_threads, sizeof(long));
    total_success = (int *) calloc(n_threads, sizeof(int));

    totalThreads = n_threads * n_cycles;

    // Check if the memory has been successfully 
    if (total_bytes == NULL) {
        printf("Memory (total_bytes) not allocated.\n");
        exit(0);
    }

    // Check if the memory has been successfully 
    if (total_ms == NULL) {
        printf("Memory (total_ms) not allocated.\n");
        exit(0);
    }
    // Check if the memory has been successfully 
    if (first_request_msec == NULL) {
        printf("Memory (first_request_msec) not allocated.\n");
        exit(0);
    }
    // Check if the memory has been successfully 
    if (total_success == NULL) {
        printf("Memory (total_success) not allocated.\n");
        exit(0);
    }

    //Create http request
    char request[54 + strlen(filename) + strlen(ip)];
    sprintf(request, "GET /%s HTTP/1.1\nHost: %s\nUser-agent: simple-http client\n\n", filename, ip);
    pthread_t all_tid[n_threads];

    printf(ANSI_COLOR_RED "The server test is starting!" ANSI_COLOR_RESET "\n");

    for (int i = 0; i < n_threads; i++) {
        //--Arguments to send to thread
        struct arg_thread *args_send = malloc(sizeof(*args_send));
        struct arg_thread args;
        if (args_send == NULL) {
            fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
            exit(EXIT_FAILURE);
        }
        args.request = request;
        args.ip = ip;
        args.port = port;
        args.n_cycles = n_cycles;
        args.id_thread = i;
        *args_send = args;
        if (pthread_create(&all_tid[i], NULL, &thread_request, args_send) != 0) {
            printf("Error in thread creation!\n");
            free(args_send);
        }
    }

    for (int i = 0; i < n_threads; i++) { /* Wait until all threads are finished */
        pthread_join(all_tid[i], NULL);
    }
    printProgress(1.0);
    printf("\n");
    printf(ANSI_COLOR_RED "The results of test are:" ANSI_COLOR_RESET "\n");

    double mb = calculate_transfered_MB(n_threads);
    int success = calculate_successes(n_threads);
    double total_msec = calculate_total_msec(n_threads);

    printf("Total data size transfered: %.2f MB.\n", mb);
    printf("Total success request: %d.\n", success);
    printf("Total failures request: %d.\n", error_request);
    if (success > 0) {
        printf("Average transfer speed: %.2f MB/seg\n", mb / (total_msec / 1000.0));
        printf("Average duration : %.2f ms\n", total_msec / (double) success);
        printf("Average speed of request acceptance:  %.2f ms\n",
               calculate_total_acceptance_msec(n_threads) / (double) success);
    } else {
        printf("Not stats as not a single request succeed.\n");
    }


    free(total_bytes);
    free(first_request_msec);
    free(total_success);
    free(total_ms);
    sem_destroy(&mutex);
    return 0;
}
