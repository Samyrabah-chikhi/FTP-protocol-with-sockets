#include "csapp.h"
#include "errno.h"

#define MAX_NAME_LEN 256
#define NPROC 2

// question 1
typedef enum{
	GET = 0,
	PUT = 1,
	LS = 2
}typereq_t;

// question 2
typedef struct request_t{
	typereq_t type;
	char *filename;
}request_t;

typedef struct{
	char ip[256];
	int port;
	int fd;
}slave_info_t;

void file_transfer(int connfd, int pid, char* foldername, int folderlen);

void handler_zombie(int sig){
 while( waitpid(-1,NULL,WNOHANG) > 0);
}

void handler_kill(int sig){
    kill(0, SIGINT);
    exit(0);
}

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    char foldername[] = "server_path/";
    int folderlen = sizeof(foldername)/sizeof(char);
    int port, i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    int pid = getpid();
    printf("----------Slave server %d started----------\n",pid);

    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    signal(SIGCHLD, handler_zombie);
    signal(SIGINT, handler_kill);
    signal(SIGPIPE, SIG_IGN); // to not crash the server when client disconnects

    // connection to server
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Close(connfd);
    printf("Server information sent to MASTER\n");

    for(i=0; i < NPROC ; i++){
    	pid_t pid = fork();
	if( pid == 0){
    		while (1) {
		printf("\n");
        	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		pid = getpid();

        	Getnameinfo((SA *) &clientaddr, clientlen,
                	    client_hostname, MAX_NAME_LEN, 0, 0, 0);

        	Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                	  INET_ADDRSTRLEN);

        	printf("server %d connected to %s (%s)\n",pid, client_hostname,
               		client_ip_string);

        	file_transfer(connfd, pid, foldername, folderlen);

		Close(connfd);

		printf("server %d disconnected to %s (%s)\n",pid, client_hostname,
                	client_ip_string);
		}
	}
    }
    while(1){
	pause();
    }
    exit(0);
}

