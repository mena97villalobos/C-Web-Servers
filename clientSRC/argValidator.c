#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int validate_number(char *str) {
    while (*str) {
        if (!isdigit(*str)) { //if the character is not a number, return false
            return 0;
        }
        str++; //point to next character
    }
    return 1;
}

int validate_ip(char *ip) { //check whether the IP is valid or not
    int i, num, dots = 0;
    char *ptr;
    if (ip == NULL)
        return 0;
    ptr = strtok(ip, "."); //cut the string using dor delimiter
    if (ptr == NULL)
        return 0;
    while (ptr) {
        //check whether the sub string is holding only number or not
        if (!validate_number(ptr))
            return 0;
        num = atoi(ptr); //convert substring to number
        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, "."); //cut the next part of the string
            if (ptr != NULL)
                dots++; //increase the dot count
        } else
            return 0;
    }
    if (dots != 3) //if the number of dots are not 3, return false
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
