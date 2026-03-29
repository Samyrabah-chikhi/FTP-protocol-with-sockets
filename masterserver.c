#include "csapp.h"
#include "errno.h"

#define MAX_NAME_LEN 256
#define PORT 2121
#define NB_SLAVES 2
#define PORT_SLAVE 5000

typedef struct server_info_t{
	char ip[256];
	int port;
}server_info_t;

typedef struct{
        server_info_t info;
        int fd;
}slave_info_t;


int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    char foldername[] = "server/";
    int i, chosen = 0;

    slave_info_t slaves[NB_SLAVES];
    int pid = getpid();

    printf("----------MASTER Server %d Started----------\n",pid);

    int result = mkdir(foldername, 0777);
    if( result == 0 ) {
        printf("Folder %s created succesfully\n",foldername);
    }
    else if( errno == EEXIST ){
        printf("Folder %s exists already\n",foldername);
    }
    else{
        printf("Error creating folder %s for server\n",foldername);
    }
    printf("\n");

    printf("Connecting to slave servers\n");
    for( i=0; i < NB_SLAVES; i++){

        slaves[i].info.port = PORT_SLAVE + i;
        strcpy(slaves[i].info.ip,"localhost");
        printf("Slaves[%d]: %s port %d\n",i+1,slaves[i].info.ip,slaves[i].info.port);

        slaves[i].fd = Open_clientfd(slaves[i].info.ip,slaves[i].info.port);
        Close(slaves[i].fd); // not needed
    }

    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(PORT);
    server_info_t chosenServer;
    while(1){
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

                Getnameinfo((SA *) &clientaddr, clientlen,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);

                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                          INET_ADDRSTRLEN);

                printf("MASTER Server %d connected to %s (%s)\n",pid, client_hostname,
                        client_ip_string);

		if( chosen == NB_SLAVES)
			chosen = 0;

		chosenServer.port = slaves[chosen].info.port;
		strcpy(chosenServer.ip, slaves[chosen].info.ip);

		Rio_writen(connfd, &chosenServer, sizeof(chosenServer));

                Close(connfd);
		chosen++;

                printf("MASTER Server %d sent server infos and disconnected to %s (%s)\n",pid, client_hostname,
                        client_ip_string);

	}

    exit(0);
}

