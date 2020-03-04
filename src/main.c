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
#include <openssl/md5.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "net.h"
#include "mime.h"

#define PORT "8080"
#define SERVER_FILES "./serverfiles"
#define SERVER_ROOT "./serverroot"

struct file_data
{
    int size;
    void *data;
};

char *find_start_of_body(char *header)
{
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

void dividir_request_path(char *request_divided[], char request_path[])
{
    request_divided[0] = strtok(request_path, "?");
    int i = 0;
    while (request_divided[i] != NULL)
    {
        i++;
        request_divided[i] = strtok(NULL, "?");
    }
}

int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{

    char *response = calloc(content_length + 1024, sizeof(char));
    time_t t1 = time(NULL);
    struct tm *ltime = localtime(&t1);
    int response_length = sprintf(response, "%s\nDate: %sConnection: close\nContent-Length: %d\nContent-Type: %s\n"
                                            "\n",
                                  header, asctime(ltime), content_length, content_type);
    memcpy(response + response_length, body, content_length);
    ssize_t rv = send(fd, response, response_length + content_length, 0);
    free(response);
    if (rv < 0)
    {
        perror("send");
    }
    return (int)rv;
}

struct file_data *file_load(char *filename)
{
    char *buffer, *p;
    struct stat buf;
    int bytes_read, bytes_remaining, total_bytes = 0;

    int res = stat(filename, &buf);

    // Get the file size
    if (res == -1)
    {
        return NULL;
    }

    // Make sure it's a regular file
    if (!(buf.st_mode & S_IFREG))
    {
        return NULL;
    }

    // Open the file for reading
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        return NULL;
    }

    // Allocate that many bytes
    bytes_remaining = buf.st_size;
    p = buffer = calloc(bytes_remaining + 1, sizeof(char));

    if (buffer == NULL)
    {
        return NULL;
    }

    // Read in the entire file
    while (bytes_read = fread(p, 1, bytes_remaining, fp), bytes_read != 0 && bytes_remaining > 0)
    {
        if (bytes_read == -1)
        {
            free(buffer);
            return NULL;
        }

        bytes_remaining -= bytes_read;
        p += bytes_read;
        total_bytes += bytes_read;
    }

    // Allocate the file data struct
    struct file_data *filedata = malloc(sizeof *filedata);

    if (filedata == NULL)
    {
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
void file_free(struct file_data *filedata)
{
    free(filedata->data);
    free(filedata);
}

int get_file_or_cache(int fd, char *filepath)
{
    struct file_data *filedata;
    char *mime_type;
    filedata = file_load(filepath);
    if (filedata == NULL)
    {
        return -1;
    }
    mime_type = mime_type_get(filepath);
    send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);
    file_free(filedata);
    return 0;
}

void get_file(int fd, char *request_path, char *puerto)
{
    char filepath[4096];
    int status;

    sprintf(filepath, "%s%s", SERVER_ROOT, request_path);
    status = get_file_or_cache(fd, filepath);
    if (status == -1)
    {
        // Return failure to client resp_404(fd);
        return;
    }
}

int handle_http_request(int fd, char *puerto)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];
    char *p;
    char request_type[8];
    char request_path[1024];
    char request_protocol[128];
    char request_path_copy[1024];
    char *request_path_div[80];
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0)
    {
        perror("recv");
        return 1;
    }
    if (bytes_recvd > 0)
    {
        request[bytes_recvd] = '\0';
        p = find_start_of_body(request);
        if (p == NULL)
        {
            printf("Could not find end of header\n");
            exit(1);
        }
        char *body = p;
        sscanf(request, "%s %s %s", request_type, request_path, request_protocol);
        strcpy(request_path_copy, request_path);
        dividir_request_path(request_path_div, request_path_copy);

        if (strcmp(request_type, "GET") == 0)
        {
            get_file(fd, request_path, puerto);
        }
        else if (strcmp(request_type, "POST") == 0)
        {
            // HANDLE POST REQUEST
        }
        else
        {
            fprintf(stderr, "unknown request type \"%s\"\n", request_type);
        }
        return 1;
    }
    else
    {
        return 1;
    }
}

int main(void)
{
    int newfd;
    int newfdmodify;
    struct sockaddr_storage their_addr;
    struct sockaddr_storage addr_modify;
    char s[INET6_ADDRSTRLEN];

    printf("1\n");
    int listenfd = get_listener_socket(PORT);
    printf("2\n");
    if (listenfd < 0)
    {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }
    while (1)
    {
        socklen_t sin_size = sizeof their_addr;
        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd == -1)
        {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        handle_http_request(newfd, PORT);
        close(newfd);
    }
    return 0;
}