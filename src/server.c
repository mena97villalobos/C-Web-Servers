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
#include <sys/stat.h>
#include "server.h"

#define SERVER_FILES "../src/serverfiles"
#define SERVER_ROOT "../src/serverroot"

int send_response(int fd, void *body, int content_length){
    char* response = calloc(content_length, sizeof(char));
    memcpy(response, body, content_length);
    ssize_t rv = send(fd, response, content_length, 0);

    free(response);
    if (rv < 0) {
        perror("send");
    }
    return (int)rv;
}



int get_file(int fd, char *filename) {
    struct file_data *filedata;
    char filepath[1017];
    sprintf(filepath, "%s/%s", SERVER_ROOT, filename);

    filedata = file_load(filepath);
    if (filedata == NULL) {
        return -1; // failure
    }

    send_response(fd, filedata->data, filedata->size);
    file_free(filedata);

    return 0; // success
}

/**
 * Loads a file into memory and returns a pointer to the data.
 *
 * Buffer is not NUL-terminated.
 */
struct file_data *file_load(char *filename) {
    char *buffer, *p;
    struct stat buf;
    int bytes_read, bytes_remaining, total_bytes = 0;

    int res = stat(filename, &buf);

    // Get the file size
    if (res == -1) {
        return NULL;
    }

    // Make sure it's a regular file
    if (!(buf.st_mode & S_IFREG)) {
        return NULL;
    }

    // Open the file for reading
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        return NULL;
    }

    // Allocate that many bytes
    bytes_remaining = buf.st_size;
    p = buffer = calloc(bytes_remaining + 1, sizeof(char));

    if (buffer == NULL) {
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
void file_free(struct file_data *filedata)
{
    free(filedata->data);
    free(filedata);
}