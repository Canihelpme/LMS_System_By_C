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
#include <stdbool.h>

#define PORT 3000
#define MSG_SIZE 1024  

typedef struct
{
    int sock;
    struct sockaddr address;
    int addr_len;

} connection_t;

struct Books {
	int id;
    char name[60];
	char a_name[60];
	char a_address[100];
    char p_date[100];
};

struct Books bk;
int first = 0;
char* add(char msgToServer[MSG_SIZE], int sock);
char* search(char msgToServer[MSG_SIZE],int sock);
struct Books storeDataInBooksStruct(char line[]);

int main(int argc, char const *argv[]) 
{
	
    int sock = 0; 

        struct sockaddr_in serv_addr; 

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        { 
            printf("\nSocket creation error \n"); 
            return -1; 
        } 

        serv_addr.sin_family = AF_INET; 

        serv_addr.sin_port = htons(PORT); 
        
        
        if(inet_pton(AF_INET, "211.108.190.47", &serv_addr.sin_addr)<=0) 
        { 
            printf("\nInvalid address or address not supported \n"); 
            return -1; 
        }
   if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
            printf("\nConnection Failed \n"); 
            return -1; 
        } 
        char serverAddressStr[sizeof(serv_addr)];
        inet_ntop(AF_INET, &(serv_addr.sin_addr), serverAddressStr, sizeof(serv_addr));
        printf("\nConnected to server at %s:%d \n",serverAddressStr, ntohs(serv_addr.sin_port)); 


    while(1)
    {
        printf("\n=====================WELCOME TO BOOK MANAGEMENT SYSTEM=====================\n");
        char msgToServer[MSG_SIZE]; 
        char msgFromServer[MSG_SIZE];
    
        int select = 0;
        printf("Select the number.\n");
        printf("1.Add\n2.Update\n3.Search\n4.Display\n5.Delete\n6.Terminate\n: ");
        scanf("%d", &select);

	    char selectnum[10];
	    sprintf(selectnum,"%d",select);
	    strcat(msgToServer, selectnum);
	    write(sock, msgToServer, sizeof(msgToServer));
        memset(msgToServer, 0, sizeof(msgToServer));
	    read(sock, msgFromServer, sizeof(msgFromServer));
	    

        if(select == 1)
        {
            char msgToServer[MSG_SIZE]; 
            char msgFromServer[MSG_SIZE];
            add(msgToServer,sock);
            write(sock, msgToServer, sizeof(msgToServer));
            read(sock, msgFromServer, sizeof(msgFromServer));

        }
        else if(select == 2)
        {	
	
			char cutword[1]; //
	
			char check[1];
	
			char *nodata ="No Book Information that you want\n";
            char msgToServer[MSG_SIZE]; 
            char msgFromServer[MSG_SIZE];
            search(msgToServer, sock);
            write(sock, msgToServer, sizeof(msgToServer));
            read(sock, msgFromServer, sizeof(msgFromServer));
			strncpy(cutword, msgFromServer,1);
	
			char *state = strtok(msgFromServer,cutword);
	
            if(!strcmp(nodata,state)){
			printf("%s",state);
			continue;
		}
	
			add(msgToServer, sock);
            write(sock, msgToServer, sizeof(msgToServer));
            read(sock,msgFromServer, sizeof(msgFromServer));
        }
        else if(select == 3)
        {
			char *nodata ="No Book Information that you want\n";
            char msgToServer[MSG_SIZE]; 
            char msgFromServer[MSG_SIZE];
            search(msgToServer, sock);
            write(sock, msgToServer, sizeof(msgToServer));
            read(sock, msgFromServer, sizeof(msgFromServer));
			if(!strcmp(msgFromServer, nodata)){
			printf(msgFromServer);
			}
			else{
					char *ptr = strtok(msgFromServer, "\n");
           			while(ptr != NULL)
            		{
             	   bk = storeDataInBooksStruct(ptr);
             	   printf("\n\n");
             	   printf("\nBook id : %d", bk.id);
             	   printf("\nBook name : %s", bk.name);
             	   printf("\nAuthor name : %s", bk.a_name);
             	   printf("\nAuthor address: %s", bk.a_address);
             	   printf("\nPublished date : %s", bk.p_date);
             	   printf("\n\n");
             	   ptr = strtok(NULL, "\n"); 
            		}	
	}
        }
        else if(select == 4)
        {
	first = 1;
            FILE* fBookPtr;
	fBookPtr = fopen("ReadBook.txt", "a+");
	if (fBookPtr == NULL) 
	{
		perror("Error! opening file");
	}
	
	
	
	fprintf(fBookPtr,"%s",msgFromServer);
	
	
	char line[300];
	rewind(fBookPtr);
	fseek(fBookPtr, 0, SEEK_END);
	int filesize = ftell(fBookPtr);
            if(filesize < 2){
		printf("\nThere is NO Data!\n");
	}else{
		rewind(fBookPtr);
		while(fgets(line, 300, fBookPtr) != NULL)
            		{	
             	   bk = storeDataInBooksStruct(line);
             	   printf("\n\n");
             	   printf("\nBook id : %d", bk.id);
             	   printf("\nBook name : %s", bk.name);
             	   printf("\nAuthor name : %s", bk.a_name);
             	   printf("\nAuthor address: %s", bk.a_address);
             	   printf("\nPublished date : %s", bk.p_date);
             	   printf("\n\n");
            	  	}
	}
	fclose(fBookPtr);
            printf("\nRead and display operations completed. \n");
	if(chmod("ReadBook.txt",0777) < 0){
		perror("chmod ERROR");
	}
	int flag = remove("ReadBook.txt");
	if (flag != 0) perror("Unable to delete file ");
	
        }
        else if(select == 5){}
        else if(select == 6)
        {
            break;
        }
    }

    return 0; 
}

char* add(char msgToServer[MSG_SIZE], int sock)
{
   printf("\nEnter Book ID : ");
   scanf("%d", &bk.id);
   getchar();
	
   printf("\nEnter Book name : ");
   scanf("%[^\n]s",&bk.name);
   getchar();

   printf("\nEnter Author name : ");
   scanf("%[^\n]s",&bk.a_name);
   getchar();
   
   printf("\nEnter Author e-mail address : ");
   scanf("%[^\n]s",&bk.a_address);
   getchar();  

   printf("\nEnter Publisher date : ");
   scanf("%[^\n]s",&bk.p_date);
   getchar();
   
   sprintf(msgToServer, "%d %s %s %s %s\n", bk.id ,bk.name ,bk.a_name, bk.a_address ,bk.p_date);
   return msgToServer;
}

char* search(char msgToServer[MSG_SIZE],int sock)
{
    scanf("%d", &bk.id);
    getchar();
    sprintf(msgToServer, "%d", bk.id);
    return msgToServer;
}

struct Books storeDataInBooksStruct(char line[]) 
{

	if(first==1)
	{
		char cutword[1];
		strncpy(cutword, line,1);
		line = strtok(line,cutword);
		
		first = 0;
	}	
	char seps[] = " ";
    char* token = NULL;

	
	token = strtok(line, seps);
	char buf[10];
	bk.id = atoi(token);

	token = strtok(NULL, seps);
	strcpy(bk.name, token);
	
	token = strtok(NULL, seps);
	strcpy(bk.a_name, token);
	
	token = strtok(NULL, seps);
	strcpy(bk.a_address, token);
	
	token = strtok(NULL, seps);
	strcpy(bk.p_date, token);

	token = strtok(NULL, seps);
		
	return bk;	
}


