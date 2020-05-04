#ifndef SERVER_PREDEFINE_FORKS_COMMONHTTPFUNCTIONS_H
#define SERVER_PREDEFINE_FORKS_COMMONHTTPFUNCTIONS_H

void errExit(const char *);

char *find_start_of_body(char *);

void divide_request_path(char **, char *);

int send_response(int, char *, char *, void *, unsigned long);

struct file_data *file_load(char *);

void file_free(struct file_data *);

void get_file(int, char *);

int handle_http_request(int);


int server_stopped(void);

#endif //SERVER_PREDEFINE_FORKS_COMMONHTTPFUNCTIONS_H
