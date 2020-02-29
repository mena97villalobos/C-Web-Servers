#ifndef SERVER_H
#define SERVER_H

struct file_data {
    int size;
    void *data;
};

int send_response(int, void *, int);
int get_file(int, char *);
struct file_data *file_load(char *);
void file_free(struct file_data *);

#endif /* SERVER_H*/