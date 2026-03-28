/*
 * client.c - An echo client
 */
#include "csapp.h"
#include "errno.h"

#define PORT 2121

int main(int argc, char **argv)
{
    int clientfd, n;
    char *host, buf[MAXLINE];
    rio_t rio;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);
    char foldername[] = "client/";

    if (argc < 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    int result = mkdir(foldername, 0777);
    if( result == 0 ) {
	printf("Folder %s created succesfully\n",foldername);
    }
    else if( errno == EEXIST ){
	printf("Folder %s exists already\n",foldername);
    }
    else{
	printf("Erreur creating the folder %s for client\n",foldername);
    }
    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, PORT);
    getpeername(clientfd, ( SA * )&clientaddr, &clientlen);
    printf("numero de port du serveur: %d\n",ntohs(clientaddr.sin_port));
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection	
     */ 
    getsockname(clientfd, ( SA * )&clientaddr, &clientlen);
    printf("Client connecté via port n°: %d\n\n",ntohs(clientaddr.sin_port));
    
    Rio_readinitb(&rio, clientfd);

    if(Fgets(buf, MAXLINE, stdin) != NULL) {

     	Rio_writen(clientfd, buf, strlen(buf));
	char command[4];
      	char filename[256];

    	sscanf(buf, "%4s %255s", command, filename);
        
	int filelen = sizeof(filename)/sizeof(char);
    	int folderlen = sizeof(foldername)/sizeof(char);

    	char path[ filelen+folderlen ];
    	strcpy(path,foldername);
	strcpy(path+folderlen-1,filename);
	
	printf("File for client: %s\n",path);
	FILE* fptr = fopen(path, "wb");

        while (( n = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
            Fputs(buf, stdout);
	    fwrite(buf, 1, n, fptr);
        }
	fclose(fptr);
    }
    Close(clientfd);
    exit(0);
}
