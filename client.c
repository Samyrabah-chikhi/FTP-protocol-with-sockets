/*
 * client.c - An echo client
 */
#include "csapp.h"
#include "errno.h"

#define PORT 2121
#define MAX_LEN 256

typedef enum{
        SUCCESS = 0,
        FAIL = 1
}result_t;

typedef enum{
        GET = 0,
        PUT = 1,
        BYE = 2,
        LS = 3,
	RM = 4
}typereq_t;

typedef struct response_t{
        result_t result;
        long long length;
}response_t;

typedef struct request_t{
        typereq_t type;
        char filename[MAX_LEN];
	int filelen;
        long long offset;
}request_t;

typedef struct server_info_t{
        char ip[256];
        int port;
}server_info_t;

#define BLOCK_SIZE 512

int main(int argc, char **argv)
{
    int clientfd, n;
    char *host, buf[MAXLINE];
    struct sockaddr_in clientaddr;
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);
    char foldername[] = "client_path/";

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

    clientfd = Open_clientfd(host, PORT);

    getpeername(clientfd, ( SA * )&clientaddr, &clientlen);
    printf("Connected to MASTER Server %s port %d\n",host,ntohs(clientaddr.sin_port));

    getsockname(clientfd, ( SA * )&clientaddr, &clientlen);
    printf("From Client port: %d\n\n",ntohs(clientaddr.sin_port));

    server_info_t slave;
    Rio_readn(clientfd, &slave, sizeof(slave));
    Close(clientfd);

    clientfd = Open_clientfd(slave.ip, slave.port);

    getpeername(clientfd, ( SA * )&clientaddr, &clientlen);
    printf("Connected to SLAVE Server %s port %d ",host,ntohs(clientaddr.sin_port));

    getsockname(clientfd, ( SA * )&clientaddr, &clientlen);
    printf("From Client port: %d\n\n",ntohs(clientaddr.sin_port));

    printf("Enter new command: ");
    request_t req;
    while( Fgets(buf, MAXLINE, stdin) != NULL) {

	if(strlen(buf) == 0 ){
		printf("Typed blank\n");
		break;
	}

	char command[16];
        char filename[256];

	if (sscanf(buf, "%16s %255s", command, filename) >= 1) {

        if (strcasecmp(command, "GET") == 0) {
                req.type = GET;
        }
        else if (strcasecmp(command, "PUT") == 0) {
                req.type = PUT;
        }
        else if(strcasecmp(command, "LS") == 0){
                req.type = LS;
        }
        else if(strcasecmp(command, "BYE") == 0){
                req.type = BYE;
        }
	else{ 	// command not recognized
		req.type = -1;
	  }
	}


        int filelen = strlen(filename);
        int folderlen = strlen(foldername);

        char path[ filelen+folderlen ];
        strcpy(path,foldername);
        strcpy(path+folderlen,filename);

	FILE* fptr = fopen(path, "rb");
	if( fptr == NULL){
		req.offset = 0;
	}
	else{
		fseek(fptr,0,SEEK_END);
		req.offset = ftell(fptr);
		fseek(fptr,0,SEEK_SET);
		fclose(fptr);
	}

	req.filelen = filelen;
	strcpy(req.filename,filename);

     	Rio_writen(clientfd, &req, sizeof(req));

	if( req.type == BYE ){
                break;
	}

	printf("Request_format{\nFile name: %s,\n-File offset: %llu,\n-Type number: %d,\n}\n",req.filename,req.offset,req.type);

	response_t res;
	if( req.type == GET || req.type == LS ||  req.type == -1 ){
		Rio_readn(clientfd, &res, sizeof(res));
	}
	else if( req.type == PUT || req.type == RM){
		//send server the file size
	}
	char body[BLOCK_SIZE];
	long long left = res.length;

	if( res.result == FAIL){
		Rio_readn(clientfd, body, res.length);
		body[res.length] = '\0';
                printf("Server: %s\n",body);
	}
	else{
		if( req.type == GET ) {
			fptr = fopen(path,"ab");
			while (left > 0) {
				int chunk = ( left > BLOCK_SIZE ) ? BLOCK_SIZE : left;
				n = Rio_readn(clientfd, body, chunk);
       				fwrite(body, 1, n, fptr);
				left = left - n;
        		}
        		fclose(fptr);
			printf("Transfer succesfully complete.\n");
		}
		else if( req.type == LS ) {
			fptr = fopen("client_path/ls","wb");
                        while (left > 0) {
                                int chunk = ( left > BLOCK_SIZE ) ? BLOCK_SIZE : left;
                                n = Rio_readn(clientfd, body, chunk);
                                fwrite(body, 1, n, fptr);
                                left = left - n;
                        }
                        fclose(fptr);
			printf("Transfer succesfully complete.\n");
                }
		else{
			printf("Not implemented by server yet\n");
		}
	}
	printf("\nEnter new command: ");
    }
    Close(clientfd);
    exit(0);
}
