#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/stat.h>
#include "../headers/mime.h"

#define SERVER_ROOT "../../serverroot"

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
            // Assume that POST and other request types are ignored
            fprintf(stderr, "unknown request type \"%s\"\n", request_type);
            return 1;
        }
    } else {
        return 1;
    }
}
