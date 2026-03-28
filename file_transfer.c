/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include "string.h"

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
        char *filename;
}request_t;

// question 6

typedef enum{
	SUCCESS = 0,
	FAIL = 1
}result_t;

typedef struct response_t{
	result_t result;
	int length;
}response_t;

void file_transfer(int connfd,int pid, char* foldername, int folderlen)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    FILE* file;
    Rio_readinitb(&rio, connfd);

    n = Rio_readlineb(&rio, buf, MAXLINE);
    buf[n-1] = '\0';
    n--;
    printf("\nserver %d received %u bytes\n",pid, (unsigned int)n);
    printf("Client request: %s\n",buf);
    
    request_t req;
    response_t response;
    char command[4];
    char filename[256];

    if (sscanf(buf, "%4s %255s", command, filename) >= 1) {

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
    else {
        
	char buff[] = "Invalid command\n";
	response.result = FAIL;
	response.length = strlen(buff);

        Rio_writen(connfd, &response, sizeof(response));
	Rio_writen(connfd, buff, response.length); 
	return ;
       }
    }
    
    int filelen = sizeof(filename)/sizeof(char);
    
    char path[ filelen+folderlen ];
    strcpy(path,foldername);
    strcpy(path+folderlen-1,filename);

    printf("path name: %s\n", path);

    file = fopen(path,"rb");
    if( file == NULL){
        char buff[] = "No file with such a name found\n";
        response.result = FAIL;
        response.length = strlen(buff);

        Rio_writen(connfd, &response, sizeof(response));
        Rio_writen(connfd, buff, response.length);
    }
    else{
	// recherche de la taille du buffer pour lire le fichier
	if( req.type == GET ){
		fseek(file, 0, SEEK_END);
		n = ftell(file);
		fseek(file, 0, SEEK_SET);

		char body[n];
		if(fread(body, 1, n, file) != 0){
			printf("File content: %s\n",body);
			response.result = SUCCESS;
			response.length = strlen(body);
		}
		else{
			response.result = FAIL;
			response.length = 0;
		}
		Rio_writen(connfd, &response, sizeof(response));
		Rio_writen(connfd, body, response.length);
		printf("seeked: %lu\n",n); 
		fclose(file);
	}
	else if( req.type == PUT){
		// later
	}
	else if( req.type == LS){
		//later
	}
     }
}

