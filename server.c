#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define PORT 3000
#define MSG_SIZE 1024  


void *connection_handler(void *);

typedef struct
{
    int sock;
    struct sockaddr address;
    int addr_len;

} connection_t;

struct Books {
	char name[60];
	int ID;
	char date[40];
	char author[60];
	char address[100];
	char publisher[60];
	char p_num[40];
};

int main(int argc, char const *argv[]) 
{ 
	int server_fd; 
	struct sockaddr_in serverAddress; 
	
    printf("Creating socket ...\n");
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	int opt = 1; 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

	serverAddress.sin_family = AF_INET; 

	if (inet_pton(AF_INET, "192.168.35.134", &serverAddress.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	serverAddress.sin_port = htons( PORT ); 

	
	if (bind(server_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 


    printf("Server Started ...\n");	

    connection_t * connection;	

	while (1)
	{
		connection = (connection_t *) malloc(sizeof(connection_t));
	    connection->sock = accept(server_fd, &connection->address, &connection->addr_len);

		if (connection->sock < 0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

        pthread_t threadID;

		if(pthread_create(&threadID , NULL ,  connection_handler , (void*) connection) < 0)
		{
			perror("Could not create thread");
			return 1;
		}			
	}
	return 0;
} 


void add_info(char msgFromClient[MSG_SIZE], connection_t * conn)
{
	char msgToClient[MSG_SIZE];
	FILE* fBookPtr;
	fBookPtr = fopen("Books.txt", "a+");
	if (fBookPtr == NULL) 
	{
	    perror("Error! opening file");
	}
	fprintf(fBookPtr,"%s",msgFromClient);
	printf(msgFromClient);
	fclose(fBookPtr);
	char *result2 = "Finish Add Information\n";
	strcat(msgToClient, result2);
	
	write(conn->sock , msgToClient , sizeof(msgToClient));
}

void update_info(char msgFromClient[MSG_SIZE], connection_t * conn)
{
			char msgToClient[MSG_SIZE];
			char line[300];
			char sep[] = " ";
			char *fileid = NULL;
			char *msgid = NULL;
			msgid = strtok(msgFromClient, sep);
			FILE* fBookPtr;
			fBookPtr = fopen("Books.txt", "r");

			if (fBookPtr == NULL) 
			{
				perror("Error! opening file");
			}

			int checkflag = 0;
			char cpline[300];

			while (fgets(line, 300, fBookPtr) != NULL) 
			{
				strcpy(cpline,line);
				fileid = strtok(cpline,sep);
				if(!strcmp(fileid, msgid)) // ?????? id ????????? ????????? ???????????? ?????? ??????
				{
					checkflag = 1;
					break;
				}
			}

			if(checkflag == 0) // ?????? id??? ?????? ????????? ?????? ??????
			{
				fclose(fBookPtr);
				char *result2 = "No Book Information that you want\n";
				strcat(msgToClient, result2);
				write(conn->sock , msgToClient , sizeof(msgToClient));
			}

			else // ?????? id??? ?????? ????????? ????????? ??????
			{
				char *result4 = "Input New Information\n";
				strcat(msgToClient,result4);
				write(conn->sock , msgToClient , sizeof(msgToClient));				
				char msgToClient[MSG_SIZE];
				char msgFromClient[MSG_SIZE];
				read(conn->sock, msgFromClient, sizeof(msgFromClient));
				char cpmsg[MSG_SIZE];
				strcpy(cpmsg,msgFromClient);
				rewind(fBookPtr); //?????? ????????? ?????? ????????? ????????? ?????????
				FILE* tempfBookPtr;
				tempfBookPtr = fopen("tempBooks.txt", "w");

				if (tempfBookPtr == NULL) 
				{
					perror("Error! opening file");
					exit(1);
				}

				while(fgets(line,300,fBookPtr) != NULL)
				{
					strcpy(cpline,line);
					fileid = strtok(cpline,sep);
					if(!strcmp(fileid, msgid)) 
					{	
						fprintf(tempfBookPtr,"%s",msgFromClient);
						continue;
					}
					fprintf(tempfBookPtr,"%s",line);
				}
				
				if (chmod("Books.txt", 0777) < 0)
					perror("chmod ERROR ");
				if (chmod("tempBooks.txt", 0777) < 0)
					perror("chmod ERROR ");

				fclose(fBookPtr);
				fclose(tempfBookPtr);
				int flag = remove("Books.txt");
				if (flag != 0) perror("Unable to remove ");
				int flagR = rename("tempBooks.txt", "Books.txt");
				if (flagR != 0) perror("Unable to remove ");
				char * result3 = "Finish Update Information\n";
				strcat(msgToClient, result3);
				write(conn->sock , msgToClient , sizeof(msgToClient));
			}
}

void search_info( char msgFromClient[MSG_SIZE], connection_t * conn)
{		
			char msgToClient[MSG_SIZE];
			char line[300];
			char sep1[] = " ";
			char sep2[] = " ";
			char *fileid = NULL;
			char *msgid = NULL;
			msgid = strtok(msgFromClient, sep2);
			FILE* fBookPtr;
			fBookPtr = fopen("Books.txt", "r");
			if (fBookPtr == NULL) 
            			{
				perror("Error! opening file");
			}
			int checkflag = 0;
			char cpline[300];
			while (fgets(line, 300, fBookPtr) != NULL) 
         			{
				strcpy(cpline,line);
				fileid = strtok(cpline,sep1);
				if(!strcmp(fileid, msgid))
                			{
					strcat(msgToClient, line);
					write(conn->sock , msgToClient , sizeof(msgToClient));
					checkflag = 1;
			    	}
		   	 }	
			
            fclose(fBookPtr);
            if(checkflag == 0)
            {
                char *result2 = "No Book Information that you want\n";
                strcat(msgToClient, result2);
                write(conn->sock , msgToClient , sizeof(msgToClient));
            }
}

void display_all_info(connection_t * conn)
{
	char msgToClient[MSG_SIZE];
  	char line[300];
	FILE* fBookPtr;
	fBookPtr = fopen("Books.txt", "r");
	if (fBookPtr == NULL) 
	{
		perror("Error! opening file");
	}
	while (fgets(line, 300, fBookPtr) != NULL ) 
	{
		strcat(msgToClient,line);
		printf("%s\n",msgToClient);	
	}
	write(conn->sock , msgToClient , sizeof(msgToClient));
	fclose(fBookPtr);
}

void delete_info()
{
     if(chmod("Books.txt",07777)<0)
                perror("Permission ERROR");
            int flag = remove("Books.txt");
            if(flag != 0) perror("Unable to delete file");
            FILE *fBookPtr;
            fBookPtr = fopen("Books.txt", "a+");
            fclose(fBookPtr);
}

void *connection_handler( void *clientsocket)
{
    printf("Client connected ...\n");	
    connection_t * conn;
    conn = (connection_t *) clientsocket;

    while(1)
    {
		char msgFromClient[MSG_SIZE];
		char msgToClient[MSG_SIZE];

        read(conn->sock, msgFromClient, sizeof(msgFromClient));
        printf("CLIENT: %s\n",msgFromClient );

		
		if(!strncmp(msgFromClient,"1",2))
		{
			char *result = "Input New Book Information\n";
			strcat(msgToClient, result);
			write(conn->sock , msgToClient , sizeof(msgToClient));
			msgToClient[0] = '\0';
			msgFromClient[0] = '\0';
			read(conn->sock, msgFromClient, sizeof(msgFromClient));
            add_info(msgFromClient, conn);
		}

        else if(!strncmp(msgFromClient,"2",2))
		{
			char *result = "Please Input ID for Update:)\n";
			strcat(msgToClient, result);
			write(conn->sock , msgToClient , sizeof(msgToClient));
			msgToClient[0] = '\0';
			msgFromClient[0] = '\0';
			read(conn->sock, msgFromClient, sizeof(msgFromClient));
			update_info(msgFromClient, conn);
		}

		else if(!strncmp(msgFromClient,"3",2))
		{
            char *result = "Please Input ID for Search :)\n";
			strcat(msgToClient, result);
			write(conn->sock , msgToClient , sizeof(msgToClient));
			msgToClient[0] = '\0';
			msgFromClient[0] = '\0';
			read(conn->sock, msgFromClient, sizeof(msgFromClient));
			search_info(msgFromClient, conn);
		}

        else if(!strncmp(msgFromClient, "4",2))
		{
           			display_all_info(conn);
		}

        else if(!strncmp(msgFromClient,"5",2))
	    {
			delete_info(msgToClient, conn);
			char *result = "Delete all information\n";
            strcat(msgToClient, result);
            write(conn->sock , msgToClient , sizeof(msgToClient));
	    }

        else
	    {
		    printf("\nSERVER_ ");
           	fgets(msgToClient, sizeof(msgToClient), stdin);
           	write(conn->sock , msgToClient , sizeof(msgToClient));
	    }
        
    }
printf("Client served\n");
 return 0;
}