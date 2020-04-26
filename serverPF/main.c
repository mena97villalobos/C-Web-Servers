#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include "../headers/net.h"
#include "../headers/mime.h"
#include "../headers/argValidator.h"
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/uio.h>

union fdmsg {
    struct cmsghdr h;
    char buf[CMSG_SPACE(sizeof(int))];
};

#define SERVER_ROOT "../../serverroot"

const char *help_string = "Usage: server <port> <num_forks>\n";

struct file_data {
    unsigned long size;
    void *data;
};

//Functions definition
void errExit(const char *);

char *find_start_of_body(char *);

void divide_request_path(char **, char *);

int send_response(int, char *, char *, void *, unsigned long);

struct file_data *file_load(char *);

void file_free(struct file_data *);

void get_file(int, char *);

int handle_http_request(int);

/**
 * Finds the start of the body on an HTTP request
 * @param header Header from the request
 * @return Start of the header if exists
 */
char *find_start_of_body(char *header) {
    char *p;
    p = strstr(header, "\n\n");
    if (p != NULL)
        return p;
    p = strstr(header, "\r\n\r\n");
    if (p != NULL)
        return p;
    p = strstr(header, "\r\r");
    return p;
}

// C error exit function implementation
void errExit(const char *str) {
    fprintf(stderr, "%s", str);
    exit(-1);
}

void divide_request_path(char **request_divided, char *request_path) {
    request_divided[0] = strtok(request_path, "?");
    int i = 0;
    while (request_divided[i] != NULL) {
        i++;
        request_divided[i] = strtok(NULL, "?");
    }
}

int send_response(int fd, char *header, char *content_type, void *body, unsigned long content_length) {
    char *response = calloc(content_length + 1024, sizeof(char));
    time_t t1 = time(NULL);
    int response_length = sprintf(
            response,
            "%s\nDate: %sConnection: close\nContent-Length: %lu\nContent-Type: %s\n""\n",
            header,
            asctime(localtime(&t1)),
            content_length,
            content_type
    );
    memcpy(response + response_length, body, content_length);
    ssize_t rv = send(fd, response, response_length + content_length, 0);
    free(response);
    if (rv < 0) {
        perror("send");
    }
    return (int) rv;
}

struct file_data *file_load(char *filename) {
    char *buffer, *p;
    struct stat buf;
    unsigned long bytes_read, bytes_remaining, total_bytes = 0;

    int res = stat(filename, &buf);

    // Make sure is not empty and it's a regular file
    if (res == -1 || !(buf.st_mode & S_IFREG)) {
        return NULL;
    }

    // Open the file for reading
    FILE *fp = fopen(filename, "rb");

    // Allocate that many bytes
    bytes_remaining = buf.st_size;
    p = buffer = calloc(bytes_remaining + 1, sizeof(char));

    if (fp == NULL || buffer == NULL) {
        return NULL;
    }

    // Read in the entire file
    while (bytes_read = fread(p, 1, bytes_remaining, fp), bytes_read != 0 && bytes_remaining > 0) {
        if (bytes_read == -1) {
            free(buffer);
            return NULL;
        }

        bytes_remaining -= bytes_read;
        p += bytes_read;
        total_bytes += bytes_read;
    }

    // Allocate the file data struct
    struct file_data *filedata = malloc(sizeof *filedata);

    if (filedata == NULL) {
        free(buffer);
        return NULL;
    }

    filedata->data = buffer;
    filedata->size = total_bytes;
    fclose(fp);
    return filedata;
}

/**
 * Frees memory allocated by file_load().
 */
void file_free(struct file_data *filedata) {
    free(filedata->data);
    free(filedata);
}

void get_file(int fd, char *request_path) {
    char filepath[4096];
    struct file_data *filedata;
    char *mime_type;

    sprintf(filepath, "%s%s", SERVER_ROOT, request_path);
    filedata = file_load(filepath);
    if (filedata == NULL) {
        return;
    }
    mime_type = mime_type_get(filepath);
    send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);
    file_free(filedata);
}

int handle_http_request(int fd) {
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];
    char *p;
    char request_type[8];
    char request_path[1024];
    char request_protocol[128];
    char request_path_copy[1024];
    char *request_path_div[80];
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return 1;
    }
    if (bytes_recvd > 0) {
        request[bytes_recvd] = '\0';
        printf("%s\n", request);
        p = find_start_of_body(request);
        if (p == NULL) {
            printf("Could not find end of header\n");
            exit(1);
        }
        sscanf(request, "%s %s %s", request_type, request_path, request_protocol);
        strcpy(request_path_copy, request_path);
        divide_request_path(request_path_div, request_path_copy);

        if (strcmp(request_type, "GET") == 0) {
            get_file(fd, request_path);
            return 0;
        } else {
            // Here we assume that POST and other request types are ignored
            fprintf(stderr, "unknown request type \"%s\"\n", request_type);
            return 1;
        }
    } else {
        return 1;
    }
}

void *create_shared_memory(size_t size) {
    int protection = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED | MAP_ANONYMOUS | MAP_SYNC;

    return mmap(NULL, size, protection, flags, -1, 0);
}

void *thread_request(
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
}

pthread_condattr_t getConditionalAttribute() {
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    return cattr;
}

pthread_mutexattr_t getMutexAttributes() {
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    return mattr;
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

    pthread_mutexattr_t mattr = getMutexAttributes();
    pthread_condattr_t cattr = getConditionalAttribute();

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
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    for (int i = 0; i < n_threads; i++) {
        pid = fork();
        if (pid == 0) {
            thread_request(
                    mut_allt,
                    con_allt,
                    con_server,
                    is_used,
                    listenfd
            );
            return 0;
        }
    }

    while (1) {
        socklen_t sin_size = sizeof their_addr;
        // newfd = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);

        fd_set set;
        int rv;
        FD_ZERO(&set); /* clear the set */
        FD_SET(listenfd, &set); /* add our file descriptor to the set */

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
