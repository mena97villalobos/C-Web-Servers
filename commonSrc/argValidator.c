#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int validate_number(char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

int validate_ip(char *ipOriginal) {
    int num, dots = 0;
    char *ptr;
    char ip[strlen(ipOriginal)];
    strcpy(ip, ipOriginal);

    ptr = strtok(ip, ".");
    if (ptr == NULL)
        return 0;
    while (ptr) {
        if (!validate_number(ptr))
            return 0;
        num = atoi(ptr);
        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, ".");
            if (ptr != NULL)
                dots++;
        } else
            return 0;
    }
    if (dots != 3)
        return 0;
    return 1;
}

int validate_port(char *port) {
    if (port == NULL)
        return 0;
    if (!validate_number(port))
        return 0;
    int res = atoi(port);
    return res >= 1 && res < +65535;
}
