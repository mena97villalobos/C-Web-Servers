#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h> 
//----------------------------

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#define errExit(msg)
#define BYTES 5000000
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define copyAinB(a,b)for (int i = 0; i < strlen(a); ++i){ b[i]=a[i];}


char *ROOT;
void startServer(char *);
//-----------------
int listenfd;

int main(int argc, char* argv[])
{
	//-Esto es para redireccionar el error del write en caso de que se quiebre el pipe
	signal(SIGPIPE, SIG_IGN);
	sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

	//-Esto es para la inicializacion del servidor
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;
	int numCliente;    

    //Default Values PATH = ~/ and PORT=8080
	char PORT[5];
	ROOT = getenv("PWD");
	//ROOT = "/home/luis98/Descargas/Servidor";
	strcpy(PORT,"8080");

	//Inicializacion de las variables de control

	printf("El servidor inicio en el puerto: %s%s%s.\nSu directorio virtual es: %s%s%s.\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
    // Setting all elements to -1: signifies there is no client connected
	startServer(PORT);

    // ACCEPT connections
	while (1)
	{

		addrlen = sizeof(clientaddr);
		numCliente = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);
		respond(numCliente);
	}

	return 0;
}

//--Inicialiacion del servidor
void startServer(char *port)
{
	struct addrinfo hints, *res, *p;

    // getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
    // socket and bind
    int option = 1;
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

    // listen for incoming connections
	if ( listen (listenfd, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}
}
