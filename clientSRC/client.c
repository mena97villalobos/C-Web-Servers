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
# include <semaphore.h> 
# include <time.h>

#define OUT_FILE "salida.txt"
#define BUFFER_SIZE 1024

//Bar
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

//Colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char *help_string = "Usage: client <maquina> <puerto> <archivo> <n-thread> <n-ciclos>\n";

//Test variables
double* mgby_sec;
double* first_request_sec; 
int error_request = 0;
sem_t mutex; 

//Struct for thread arguments
struct arg_thread{
    char *request; 
    char *port; 
    char *ip;
    int n_cycles;
    int id_thread;
};

//Funtions definition
char *codificar(char string[]);
char *concat(const char *s1, const char *s2);
int final_header(char string[], int maxCheck);
void substring(char [], char[], int, int);
void errExit(const char *str);
void make_request(char *request, char *port, char *ip, int id_location, int id_cycle);
void *thread_request(void *arguments);
void printProgress (double percentage);
double calculate_average_speed(int n_threads, int n_cycles);
double calculate_average_first_speed(int n_threads, int n_cycles);

//C funtion to print de progress
void printProgress (double percentage)
{
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
}

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
void make_request(char *request, char *port, char *ip, int id_location, int id_cycle){
    //Define inicitial variables
    struct addrinfo *result = NULL, hints;
    int srvfd = 0, rwerr = 42;
    char temp_buf[BUFFER_SIZE];
    char buf[BUFFER_SIZE];
    unsigned long total_bytes = 0;

    memset(temp_buf, 0, BUFFER_SIZE);    
    memset(buf, 0, BUFFER_SIZE);

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


    // Initial time first request
    clock_t begin_first = clock();

    //Send http request
    write(srvfd, request, strlen(request));

    // Turn down socket
    shutdown(srvfd, SHUT_WR);

    // Open file destination exit
    FILE *destFile = fopen(OUT_FILE, "wb");

    // Initial time
    clock_t begin = clock();
    //Check a remove the initial header
    int bytesReceived = recv(srvfd, temp_buf, BUFFER_SIZE, 0);
    clock_t end_first = clock();
    total_bytes+=bytesReceived;
    if (bytesReceived<0){
        fprintf(stderr, "%s\n","recv() failed");
        sem_wait(&mutex);
        error_request++;
        sem_post(&mutex); 
        return;
    }
    int header_final_location = final_header(temp_buf, bytesReceived);
    if (header_final_location==-1){
        fprintf(stderr, "%s\n","Error http format in the server");
        return;
    }
    substring(temp_buf,buf,header_final_location+1,bytesReceived-header_final_location+1);
    bytesReceived-=header_final_location;

    //Cliying for get all file
    while(bytesReceived != 0){
      fwrite(buf, bytesReceived, 1, destFile);
      bytesReceived = recv(srvfd, buf, BUFFER_SIZE, 0);
      total_bytes+=bytesReceived;
      if (bytesReceived<0){
        fprintf(stderr, "%s\n","recv() failed");
        sem_wait(&mutex);
        error_request++;
        sem_post(&mutex); 
        return;
      }
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    double time_spent_first = (double)(end_first - begin_first) / CLOCKS_PER_SEC;
    long double total_megaby = (double)total_bytes * 0.000001;
    double megaby_sec = total_megaby/time_spent;
    sem_wait(&mutex);
    mgby_sec[id_location+id_cycle]=megaby_sec;
    first_request_sec[id_location+id_cycle]=time_spent_first;
    sem_post(&mutex);

    //Close files and free memory
    fclose(destFile);
    close(srvfd);
}

void *thread_request(void *arguments){
    struct arg_thread* args = (struct arg_thread*) arguments;
    for (int i = 0; i < args->n_cycles; ++i)
    {
        make_request(args->request, args->port, args->ip, (int)args->id_thread*(int)args->n_cycles, i);
    }
}

double calculate_average_speed(int n_threads, int n_cycles){
    int total_ok = 0;
    long double total_speed = 0;
    for (int i = 0; i < n_threads*n_cycles; ++i)
    {
        if (mgby_sec[i]!='\0'){
            total_speed+= mgby_sec[i];
            total_ok++;
        }
    }
    if (total_ok==0){
        return 0.0;
    }
    return total_speed/(double)total_ok;
}

double calculate_average_first_speed(int n_threads, int n_cycles){
    int total_ok = 0;
    long double total_speed = 0;
    for (int i = 0; i < n_threads*n_cycles; ++i)
    {
        if (first_request_sec[i]!='\0'){
            total_speed+= first_request_sec[i];
            total_ok++;
        }
    }
    if (total_ok==0){
        return 0.0;
    }
    return total_speed/(double)total_ok;
}

int main(int argc, char **argv) {
    //Define inicitial variables
    int n_threads = 0, n_cycles = 0;
    char port[6]="", ip[16]="", filename[1000]=""; 
    sem_init(&mutex, 0, 1);

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

    //Define global variables
    mgby_sec = (double*)malloc(n_threads * n_cycles * sizeof(double));
    first_request_sec = (double*)malloc(n_threads * n_cycles * sizeof(double));

    // Check if the memory has been successfully 
    if (mgby_sec == NULL) { 
        printf("Memory (mgby_sec) not allocated.\n"); 
        exit(0); 
    } 

    // Check if the memory has been successfully 
    if (first_request_sec == NULL) { 
        printf("Memory (first_request_sec) not allocated.\n"); 
        exit(0); 
    } 

    memset(mgby_sec,'\0', n_threads * n_cycles);
    memset(first_request_sec,'\0', n_threads * n_cycles);

    //Create http request
    char request[54+strlen(filename)+strlen(ip)];
    sprintf(request,"GET /%s HTTP/1.1\nHost: %s\nUser-agent: simple-http client\n\n",filename,ip);
    pthread_t all_tid[n_threads];

    
    for (int i = 0; i < n_threads; i++) {
        struct arg_thread args;
        args.request = request;
        args.ip = ip;
        args.port = port;
        args.n_cycles = n_cycles;
        args.id_thread = i;
        if (pthread_create(&all_tid[i], NULL, &thread_request, (void *)&args) != 0) {
            printf("Error in thread creation!\n");
        }
    }

    printf(ANSI_COLOR_RED     "The server test is starting!"     ANSI_COLOR_RESET "\n");
    
    for (int i = 0; i < n_threads; i++){ /* Wait until all threads are finished */
        pthread_join(all_tid[i], NULL);
        printProgress((float)i/(float)n_threads);
    }
    printProgress(1.0);
    printf("\n");
    printf(ANSI_COLOR_RED     "The results of test are:"     ANSI_COLOR_RESET "\n");
    printf("%s %f mg/seg\n", "Average transfer speed:", calculate_average_speed(n_threads, n_cycles));
    printf("%s %f seg\n", "Average speed of request acceptance:", calculate_average_first_speed(n_threads, n_cycles));
    printf("%s %i\n", "Amount of transfers canceled:", error_request);

    free(mgby_sec);
    free(first_request_sec);
    sem_destroy(&mutex); 
    return 0;
}
