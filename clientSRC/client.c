# include <stdio.h>
# include <stdlib.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netdb.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include "argValidator.h"
# include <pthread.h>

#define OUT_FILE "salida.txt"
#define BUFFER_SIZE 1024

const char *help_string = "Usage: client <maquina> <puerto> <archivo> <n-thread> <n-ciclos>\n";

struct arg_thread{
    char *request; 
    char *port; 
    char *ip;
    int n_cycles;
};

char *codificar(char string[]);
char *concat(const char *s1, const char *s2);
int final_header(char string[], int maxCheck);
void substring(char [], char[], int, int);
void errExit(const char *str);
void make_request(char *request, char *port, char *ip);
void *thread_request(void *arguments);


//C final header function implementation, return -1 if the standar is incorrect
int final_header(char string[], int maxCheck){
    int i = 0, j = 0, flat = 0;

    while(i<maxCheck){
        if (string[i]=='\n'){
            if (j!=0 && j+1 == i){
                i+=1;
                flat = 1;
                break;
            }
            j = i;
        }
        i++;
    }
    if(flat)return i;
    return -1;
}

// C error exit function implementation
void errExit(const char *str) {
    fprintf(stderr, "%s",str);
    exit(-1);
}

// C substring functio implementation
void substring(char s[], char sub[], int p, int l) {
   int c = 0;
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
}

// Code for do request
void make_request(char *request, char *port, char *ip){
    printf("%s\n",request );
    //Define inicitial variables
    struct addrinfo *result = NULL, hints;
    int srvfd = 0, rwerr = 42;
    char *temp_buf = (char*) malloc((BUFFER_SIZE+1)*sizeof(char));
    char *buf = (char*) malloc((BUFFER_SIZE+1)*sizeof(char));
    
    // Clen variables
    memset(&hints, 0, sizeof(struct addrinfo));

    // Define internet protocol
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (0 != getaddrinfo(ip, port, &hints, &result)){
        fprintf(stderr, "%s\n","getaddrinfo");
        return;
    }

    // Create socket after retrieving the inet protocol to use (getaddrinfo)
    srvfd = socket(result->ai_family, SOCK_STREAM, 0);

    if (srvfd < 0){
        fprintf(stderr, "%s\n","socket()");
        return;
    }

    if (connect(srvfd, result->ai_addr, result->ai_addrlen) == -1) {
        fprintf(stderr, "%s\n","connect");
        return;
    }

    //Send http request
    write(srvfd, request, strlen(request));

    // Turn down socket
    shutdown(srvfd, SHUT_WR);

    // Open file destination exit
    FILE *destFile = fopen(OUT_FILE, "wb");

    //Check a remove the initial header
    int bytesReceived = recv(srvfd, temp_buf, BUFFER_SIZE, 0);
    if (bytesReceived<0){
        fprintf(stderr, "%s\n","recv() failed");
        return;
    }
    int header_final_location = final_header(temp_buf, bytesReceived);
    if (header_final_location==-1){
        fprintf(stderr, "%s\n","Error http format in the server");
        return;
    }
    substring(temp_buf,buf,header_final_location+1,bytesReceived-header_final_location+1);
    bytesReceived-=header_final_location;
    free(temp_buf);

    //Cliying for get all file
    while(bytesReceived != 0){
      fwrite(buf, bytesReceived, 1, destFile);
      bytesReceived = recv(srvfd, buf, BUFFER_SIZE, 0);
      if (bytesReceived<0){
        fprintf(stderr, "%s\n","recv() failed");
        return;
      }
    }

    //Close files and free memory
    free(buf);
    fclose(destFile);
    close(srvfd);
}

void *thread_request(void *arguments){
    struct arg_thread* args = (struct arg_thread*) arguments;
    printf("%s\n", args->request);
    for (int i = 0; i < args->n_cycles; ++i)
    {
        make_request(args->request, args->port, args->ip);
    }
}

int main(int argc, char **argv) {
    //Define inicitial variables
    int n_threads = 0, n_cycles = 0;
    char *request= NULL, port[6]="", ip[16]="", filename[1000]="";

    //Check the count of arguments
    if (argc < 6)
        errExit(help_string);

    // Validate program arguments
    if (!validate_ip(argv[1])) errExit("Invalid IP Address");
    if (!validate_port(argv[2])) errExit("Invalid port");
    if (!validate_number(argv[4])) errExit("Invalid n-threads");
    if (!validate_number(argv[5])) errExit("Invalid n-cycles");

    // Clen variables
    memset(port, 0, 6);
    memset(ip, 0, 16);
    memset(filename, 0, 1000);    

    // Copy data to variables
    strncpy(ip, argv[1], strlen(argv[1]));
    strncpy(port, argv[2], strlen(argv[2]));
    strncpy(filename, argv[3], strlen(argv[3]));
    n_threads =atoi(argv[4]);
    n_cycles =atoi(argv[5]);

    //Create http request
    request = (char*) malloc((54+strlen(filename)+strlen(ip))*sizeof(char));
    sprintf(request,"GET /%s HTTP/1.1\nHost: %s\nUser-agent: simple-http client\n\n",filename,ip);
    pthread_t all_tid[n_threads];
    printf("%s\n", request);

    struct arg_thread args;
    args.request = request;
    args.ip = ip;
    args.port = port;
    args.n_cycles = n_cycles;

    for (int i = 0; i < n_threads; i++) {
        if (pthread_create(&all_tid[i], NULL, &thread_request, (void *)&args) != 0) {
            printf("Error in thread creation!\n");
        }
    }

    for (int i = 0; i < n_threads; i++) /* Wait until all threads are finished */
        pthread_join(all_tid[i], NULL);

    free(request);

    return 0;
}
