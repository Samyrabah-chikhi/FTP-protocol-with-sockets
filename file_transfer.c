/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include "string.h"

#define MAX_LEN 256

// question 1
typedef enum{
        GET = 0,
        PUT = 1,
	BYE = 2,
        LS = 3
}typereq_t;

// question 2
typedef struct request_t{
        typereq_t type;
        char filename[MAX_LEN];
	int filelen;
	long long offset;
}request_t;

// question 6

typedef enum{
	SUCCESS = 0,
	FAIL = 1
}result_t;

typedef struct response_t{
	result_t result;
	long long length;
}response_t;

// question 8
#define BLOCK_SIZE 512

void file_transfer(int connfd,int pid, char* foldername, int folderlen)
{
    size_t n;
    rio_t rio;
    FILE* fptr;
    Rio_readinitb(&rio, connfd);


    while(1){
	request_t req;
	response_t response;

	n = Rio_readn(connfd, &req, sizeof(req));
	printf("\nserver %d received %u bytes\n",pid, (unsigned int)n);

   	if ( req.type == GET || req.type == PUT ||req.type == LS ){
	}
	else if ( req.type == BYE ){
		printf("Client: BYE\n");
        	break;
	}
    	else {
		char buff[] = "Invalid command\n";
		response.result = FAIL;
		response.length = strlen(buff);

        	Rio_writen(connfd, &response, sizeof(response));
		Rio_writen(connfd, buff, response.length); 
    	}

    	char path[ req.filelen +folderlen ];
   	strcpy(path,foldername);
    	strcpy(path+folderlen-1,req.filename);

    	fptr = fopen(path,"rb");

   	if( fptr == NULL){
        	char buff[] = "No file with such a name found\n";
        	response.result = FAIL;
        	response.length = strlen(buff);

        	Rio_writen(connfd, &response, sizeof(response));
        	Rio_writen(connfd, buff, response.length);
    	}
    	else{
		fseek(fptr, req.offset, SEEK_SET);
		// recherche de la taille du buffer pour lire le fichier
		if( req.type == GET ){
			printf("Client: GET %s at offset %llu\n",path,req.offset);
			fseek(fptr, 0, SEEK_END);
			int n = ftell(fptr) - req.offset;
			fseek(fptr, req.offset, SEEK_SET);

			response.result = SUCCESS;
                	response.length = n;
			Rio_writen(connfd, &response, sizeof(response));

			char *body = malloc( sizeof(char) *(BLOCK_SIZE + 1));
			while(( n = fread(body, 1, BLOCK_SIZE, fptr) ) != 0){
				body[n]='\0';
				if( rio_writen(connfd, body, n) < 0 ){
					printf("Client disconnected while sending data\n");
					break;
				}
			}
			fclose(fptr);
		}
		else if( req.type == PUT){
			// later
		}
		else if( req.type == LS){
			//later
		}
		printf("Succesfully handled client request\n");
     	}
    }
	return ;
}

