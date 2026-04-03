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

char* get_ip() {
    char hostbuffer[256];
    struct hostent *host_entry;
    int hostname;
    struct in_addr **addr_list;

    // retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1) {
        perror("gethostname error");
        exit(1);
    }
    printf("Hostname: %s\n", hostbuffer);

    // Retrieve IP addresses
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL) {
        perror("gethostbyname error");
        exit(1);
    }
    addr_list = (struct in_addr **)host_entry->h_addr_list;
    return inet_ntoa(*addr_list[0]);
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    char foldername[] = "server_path/";
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
    char *server_ip = get_ip();
    for( i=0; i < NB_SLAVES; i++){

        slaves[i].info.port = PORT_SLAVE + i;
        strcpy(slaves[i].info.ip,server_ip);
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

		if( chosen == NB_SLAVES)
			chosen = 0;

		chosenServer.port = slaves[chosen].info.port;
		strcpy(chosenServer.ip, slaves[chosen].info.ip);

		Rio_writen(connfd, &chosenServer, sizeof(chosenServer));

                Close(connfd);

                printf("MASTER Server %d sent Slave %d to Cient %s (%s)\n",pid,chosen, client_hostname,
                        client_ip_string);
		chosen++;
	}

    exit(0);
}

