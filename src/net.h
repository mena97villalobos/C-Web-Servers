#ifndef NET_H
#define NET_H

void *get_in_addr(struct sockaddr *sa);
int get_listener_socket(char *port);

#endif /* NET_H*/