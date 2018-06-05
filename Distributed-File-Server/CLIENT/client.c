/****************************
 * File: Distributed File System Client
 * Subject: Network Systems 
 * Author: Arundhathi Swami 
 * Date: November 6 2017 
 * *************************/ 
/*Date: 17th November 2017*/

#include<stdio.h>
#include<stdint.h> 
#include<stdlib.h> 
#include<string.h> 
#include<math.h> 
#include<unistd.h> 
#include<sys/types.h>
#include<sys/socket.h> 
#include<sys/stat.h> 
#include<netinet/in.h> 
#include<netdb.h> 
#include<fcntl.h>
#include <sys/dir.h> 

/*Macro Definitions*/
#define MAX_CONNECTIONS 4
#define BYTES 1024 
#define BUFFER_SIZE 512
#define MAX_COMMAND_BUFFER_SIZE 500
#define MAX_FILE_NAME_BUFFER_SIZE 500
#define CONNECTION_TIME_LIMIT 10 /* TIMEOUT OF 2 SWCONDS INTERVAL*/


int socketfiledescriptor[MAX_CONNECTIONS], portno, clientlength, returnvalue, newsockfd; 

void error(char* error_message) //prints out error related to system call 
{
	perror(error_message); 
	//exit(1); 
}

typedef enum ERRORS
{
	SUCCESS = 0,
	FAILURE, 
} error_t; 

typedef struct server{
	char ip[20];
	char port[6]; 
} server_t; 

typedef struct credentials{
	char username[10]; 
   char password[10]; 
} cred_t; 

error_t parseCommand(char incommand[MAX_COMMAND_BUFFER_SIZE], char filename[MAX_COMMAND_BUFFER_SIZE], char subdirectoryname[MAX_COMMAND_BUFFER_SIZE])
{
	int index = 0; 
	char* ptrin = incommand; 
	char store[MAX_COMMAND_BUFFER_SIZE];
	char storein[MAX_COMMAND_BUFFER_SIZE];
	ptrin = strtok(incommand, " \n"); 
	
	while (ptrin != NULL)
	{
		if (index == 0) 
		{
			strncpy(store, ptrin , strlen(ptrin)); 
		}
		else if (index == 1)
		{ 
			strncpy(filename, ptrin, strlen(ptrin)); 		
		}
		else 
		{
			strncpy(subdirectoryname, ptrin, strlen(ptrin)); 
		}
		index++; 
		
		ptrin = strtok(NULL, " ,-\n"); 
	
	}
	return SUCCESS; 
}

error_t parseConfFile(cred_t* cred, server_t* servers, char file[])
{
 	FILE *fp = fopen(file, "r");
	if(fp==NULL)
  	{
   	printf("Error: File Doesnt Exist\n");
  	}
  	else
  	{
   	printf("Progress: Config File Parsing Started\n");
    	char parsesegment[315];
    	char* word;
    	while(fgets(parsesegment, sizeof(parsesegment), fp))
    	{  
      	word = strtok(parsesegment, "\n");
     // printf("%s\n", word);
      	if (strncmp(word, "#server one", 11) == 0)
      	{
       		char* value;
        		char* ipad; 
        		char* portno; 
        		char partsegment[35]; 
        		fgets(partsegment, sizeof(partsegment), fp);
        		value = strtok(partsegment, "\n\t");
        		value = strtok(partsegment, "\n\t "); 
        		value = strtok(NULL, "\n\t "); 
        		ipad = strtok(NULL, "\n\t: ");
        		portno = strtok(NULL, "");
        		strcpy(servers->ip, ipad);
        		strcpy(servers->port, portno);
				servers++;      	
			}
      	if (strncmp(word, "#server two", 11) == 0)
      	{
        		char* value;
        		char* ipad; 
        		char* portno; 
        		char partsegment[35]; 
        		fgets(partsegment, sizeof(partsegment), fp);
        		value = strtok(partsegment, "\n\t");
        		value = strtok(partsegment, "\n\t "); 
        		value = strtok(NULL, "\n\t "); 
        		ipad = strtok(NULL, "\n\t: ");
        		portno = strtok(NULL, "");
        		strcpy(servers->ip, ipad);
        		strcpy(servers->port, portno); 
				servers++; 
      	}
      	if (strncmp(word, "#server three", 13) == 0)
      	{
        		char* value;
        		char* ipad; 
        		char* portno; 
        		char partsegment[35]; 
        		fgets(partsegment, sizeof(partsegment), fp);
        		value = strtok(partsegment, "\n\t");
        		value = strtok(partsegment, "\n\t "); 
        		value = strtok(NULL, "\n\t "); 
        		ipad = strtok(NULL, "\n\t: ");
        		portno = strtok(NULL, "");
        		strcpy(servers->ip, ipad);
        		strcpy(servers->port, portno);
				servers++; 
      	}
      	if (strncmp(word, "#server four", 12) == 0)
      	{
      	   char* value;
      	   char* ipad; 
        		char* portno; 
        		char partsegment[35]; 
        		fgets(partsegment, sizeof(partsegment), fp);
        		value = strtok(partsegment, "\n\t");
        		value = strtok(partsegment, "\n\t "); 
        		value = strtok(NULL, "\n\t "); 
        		ipad = strtok(NULL, "\n\t: ");
        		portno = strtok(NULL, "");
        		strcpy(servers->ip, ipad);
        		strcpy(servers->port, portno);
      	}
      	if (strncmp(word, "#username", 9) == 0)
      	{
        		char* value;
        		char partsegment[25]; 
        		fgets(partsegment, sizeof(partsegment), fp);
        		value = strtok(partsegment, "\n"); 
        		value = strtok(partsegment, ":"); 
        		value = strtok(NULL, ": ");
        		strcpy(cred->username, value);          
		  
      	}
      	if (strncmp(word, "#password", 9) == 0)
      	{
        		char* value;
        		char partsegment[25]; 
        		fgets(partsegment, sizeof(partsegment), fp);
        		value = strtok(partsegment, "\n"); 
        		value = strtok(partsegment, ":"); 
        		value = strtok(NULL, ": ");
        		strcpy(cred->password, value);          
			}
     	}//while loop ends
  } //else loop ends
  fclose(fp); 
}

size_t findFileSize(FILE *filep)
{
	fseek(filep, 0, SEEK_END); //takes file pointer to end of file 
	int flen = ftell(filep); //gets no of bytes of file 
	printf("Length of File: %d\n" , flen);
	rewind(filep); //takes fp back to beginning of file
	return flen; 
}

error_t sendToDFS(int servernumber, char* partfile, int size, int chunknumber)
{
	/*send the server number, receive ack, send the chunk number,  receive ack, send the part size, receive ack, send the chunk*/
	char csize[5]; 
	memset(csize, '\0', sizeof(csize));

	/*Sending the server number*/	
	char rcvd[MAX_COMMAND_BUFFER_SIZE];//contains command both sent and received 
	memset(rcvd, '\0', sizeof(rcvd));
	char servernu[5]; 
	sprintf(servernu, "%d", servernumber); 
	printf("Sending to Server Number: %s\n", servernu);
	int noofbytessent = send(socketfiledescriptor[servernumber], servernu, sizeof(servernu), MSG_NOSIGNAL);	
	if (noofbytessent < 0)	
	{
		error("Error: Writing to server\n"); 
	} 
 
	/*sending chunk size*/ 
	sprintf(csize, "%d", size); 
	noofbytessent = send(socketfiledescriptor[servernumber], csize, sizeof(csize), MSG_NOSIGNAL);
	if (noofbytessent <= 0)	
	{
		error("Error: Writing to server\n"); 
	} 
	

	/*sending chunk number*/
	char chunkno[10]; 
	sprintf(chunkno, "%d", chunknumber); 
	printf("Sending Chunk Number: %s\n", chunkno); 
	noofbytessent = send(socketfiledescriptor[servernumber], chunkno, sizeof(chunkno), MSG_NOSIGNAL);	
	if (noofbytessent < 0)	
	{
		error("Error: Writing to server\n"); 
	} 
	
	/*Sending the actual chunk*/ 
	char arr[size];
	memset(arr,'\0',sizeof(arr));
	for (int z = 0; z< size; z++)
	{
		arr[z] = partfile[z]; 
	}
	noofbytessent = send(socketfiledescriptor[servernumber], arr, size, MSG_NOSIGNAL);	
	printf("no of bytes sent for file = %d\n", noofbytessent); 	
	if (noofbytessent < 0)	
	{
		error("Error: Writing to server\n"); 
	} 
	memset(rcvd, '\0', sizeof(rcvd));
	return SUCCESS;
}

int main(int argc, char* argv[])
{//main begins

	if(argc < 2)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0); 
	}
	
	char command[MAX_COMMAND_BUFFER_SIZE];//contains command both sent and received 
	memset(command, '\0', sizeof(command));
	char rcvd[MAX_COMMAND_BUFFER_SIZE];//contains command both sent and received 
	memset(rcvd, '\0', sizeof(rcvd));
	char file[MAX_FILE_NAME_BUFFER_SIZE];	//accepts file name from parser function
	memset(file, '\0', sizeof(file));
	char inusername[MAX_FILE_NAME_BUFFER_SIZE]; 
   memset(inusername, '\0', sizeof(inusername));
	char inpassword[MAX_FILE_NAME_BUFFER_SIZE];
	memset(inpassword, '\0', sizeof(inpassword));
	char md5sumbuffer[100]; 
	memset(md5sumbuffer, '\0', sizeof(md5sumbuffer)); 
	char allow[3] = "yes"; 
	char stop[2]= "no"; 
	int distributiontable[4][4] = {{12,23,34,41}, {41,12,23,34}, {34,41,12,23}, {23,34,41,12}};//file distribution table 
	int partfilesize, fullfilesize, chunkone, chunktwo, lastchunksize;   
	char filesize[10], subdirectory[10], usersubdirectory[10];
	
	int noofbytesent; 
	int activeservers[4] = {0};
	volatile int nooffailedservers = 0;


	printf("hello\n");
	server_t servers[MAX_CONNECTIONS]; //structure to store list of available servers 
   cred_t authenticate; //structure to store list of authorized users 
	char conffile[200]; 
	
	if (argc == 2)
	{
		strncpy(conffile, "dfc.conf", sizeof("dfc.conf")); 
		error_t ret = parseConfFile(&authenticate, servers, conffile);
	} 
	else 
	{
		error_t ret = parseConfFile(&authenticate, servers, argv[2]);
	}
   

	struct sockaddr_in server_address[MAX_CONNECTIONS], client_address; //struct sockaddr_in is a structure that contains an internet address 

	struct hostent *server; //hostent is a structure that defines a host computer on the internet 
 	
	
	while(1)
	{//while loop
		/*authenticate user name and password at client end*/
		printf("Enter Username: \n"); 
		fgets(inusername, MAX_FILE_NAME_BUFFER_SIZE, stdin); 
		printf("Enter Password: \n"); 
		fgets(inpassword, MAX_FILE_NAME_BUFFER_SIZE, stdin); 
		if (strncmp(inusername, authenticate.username, strlen(authenticate.username)) == 0 && strncmp(inpassword, authenticate.password, strlen(authenticate.password)) == 0)
		{//authenticate
			printf("Progress: Authenticated\n"); 	
			//section to send command to server 	
		  	printf("\n\nPlease enter one of five options: \nOption 1: get [filename] [directory] \nOption 2: put [filename] [directory] \nOption 3: list [directory] \nOption 4: exit\n"); 
	  		memset(command, '\0', sizeof(command));//clears command buffer 
	  		memset(file,'\0', sizeof(file));
	  		memset(subdirectory, '\0', sizeof(subdirectory)); 
	  		fgets(command, MAX_COMMAND_BUFFER_SIZE, stdin); //accepts string from command line 
			char* ptr_to_command = command;
	 		char* ptr_to_commands;
			int ret = parseCommand(ptr_to_command, file, subdirectory);		
	  		//printf("Command: %s\n", command);
     		printf("File: %s\n", file);
	  		printf("Subdirectory: %s\n", subdirectory);
	  		strncpy(usersubdirectory, subdirectory, sizeof(subdirectory));
			//printf("User Sub Directory: %s\n", usersubdirectory);  
			if (strncmp(command, "put", strlen("put")) != 0 && strncmp(command, "get", strlen("get")) != 0 && strncmp(command, "list", strlen("list")) != 0  && strncmp(command, "exit", strlen("exit")) != 0) 
			{
				printf("Error: Command Not Supported. Try again.\n"); 
				continue; 
			}
			for (int i = 0; i< MAX_CONNECTIONS; i++)
			{
				portno = atoi(servers[i].port); //concerts all 4 port numbers to integers 
				//argc checks the port number validity and existence 	
				if ((socketfiledescriptor[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{
					error("ERROR: Couldnt Create Socket\n"); 
				}//creates the socket
				server_address[i].sin_family = AF_INET;  //assigns af_inet as the address family 
				server = gethostbyname(argv[1]); 
				if (server == NULL)
				{
					fprintf(stderr, "ERROR: No such host found\n"); 
				}
		
				bcopy((char*)server->h_addr, (char*)&server_address[i].sin_addr, server->h_length); 
	
				server_address[i].sin_port = htons(portno); //assigns port number to appropriate structure member converting it from host name to network resolvable name 
   
				if (connect(socketfiledescriptor[i], (struct sockaddr *)&server_address[i], sizeof(server_address[i]))<0)
				{
					error("Error: Connecting to the server socket\n"); 
					activeservers[i] = 1; 
					nooffailedservers++;
				}
	   		clientlength = sizeof(struct sockaddr_in);
				//setting timeout of 1s
				struct timeval time; 
				time.tv_sec = CONNECTION_TIME_LIMIT; 
				time.tv_usec = 0;
	
				if(setsockopt(socketfiledescriptor[i], SOL_SOCKET, SO_RCVTIMEO, (char *) &time, sizeof(time)) < 0)
				{
					printf("Error: Setting socket options\n"); 
				}
   		}//end of socket creation for all 4 servers 
		   printf("No of Failed Server Connections: %d\n", nooffailedservers); 
			int noofacks = 0; 
			for(int u = 0; u<MAX_CONNECTIONS; u++)
	 		{//authentication process begins for loop 
				if(activeservers[u] == 1)
				{
					printf("Error: Not Sending to Inactive Server %d\n", u); 
					continue; 
				}
		   	/*sending credentials for authenctication*/	
				noofbytesent = send(socketfiledescriptor[u], &authenticate, sizeof(authenticate), MSG_NOSIGNAL);	
		   	if (noofbytesent < 0)	
	  			{
					error("Error: Writing to server\n"); 
  	  			}
        		
				int noofbytesreceived = recv(socketfiledescriptor[u], rcvd, sizeof(rcvd), 0); 
				printf("%s\n", rcvd);
				if (strncmp("no", rcvd, 2) == 0)
				{
					printf("Error: Credentials Failed Authorization. Try again\n"); 
					memset(command, '\0', sizeof(command));//clears command buffer 
				}
			
				else if(strncmp("yes", rcvd, 3) == 0)
				{
					printf("Progress: Credentials Matched\n");
					noofacks += 1;  

				}
			}//authenticating all four servers
			if (noofacks == 4) //if all servers authenticate users 
			{
				printf("Progess: All servers authorized\n");
			}

			/*PUT COMMAND*/
			if (strcmp("put", command) == 0)
     		{
				FILE *fp; 
	  			fp = fopen(file, "rb"); 
	  			if (fp == NULL)
	  			{
				  printf("Error: Invalid File. Try Again\n");
				  continue; 
				}
	 			else if (fp != NULL)
	  			{//valid file 
					/*creates md5sum of file*/
					strcpy(subdirectory, "."); 
					char md5command[] = "md5sum"; 
					sprintf(md5command, "md5sum %s >md5sumsave.txt", file); 
					printf("The cli command is: %s\n", md5command); 
					system(md5command); 
					FILE *fptr = fopen("md5sumsave.txt", "r"); 
					for( int  u = 0; u< MAX_CONNECTIONS; u++)
					{
						if(activeservers[u] == 1)
						{
							continue; 
						}
						/*sends command name*/ 
						noofbytesent = send(socketfiledescriptor[u], command, sizeof(command), MSG_NOSIGNAL);	
						if (noofbytesent < 0)	
	  					{
							error("Error: Writing to server\n"); 
  	  					}
						if ( fptr == NULL)
						{
							printf("Error: Cant send file\n"); 
							continue; 
						}
						else 
						{
							/*sends file name*/
							noofbytesent = send(socketfiledescriptor[u], file, sizeof(file), MSG_NOSIGNAL);
							if (noofbytesent < 0)	
	  						{
								error("Error: Writing to server\n"); 
  	  						}
							/*sends usersubdirectory*/
							printf("Send karte time the subdirectory is: %s\n", usersubdirectory);
							noofbytesent = send(socketfiledescriptor[u], usersubdirectory, sizeof(usersubdirectory), MSG_NOSIGNAL);
							if (noofbytesent < 0)	
	  						{
								error("Error: Writing to server\n"); 
  	  						}
							printf("NO of bytes sent for subdirectory%d\n", noofbytesent); 
						
						}//else ends 
					
					}//for ends
					memset(usersubdirectory, '\0', sizeof(usersubdirectory));
					fclose(fptr);
					fptr = fopen("md5sumsave.txt", "r"); 						
					fgets(md5sumbuffer, sizeof(md5sumbuffer), fptr);
					char md5sum[32], md5last, md5; 
					int servernumber; 
					strncpy(md5sum, md5sumbuffer, 32); //gets the 32 byte md5sum from the m5dsum text file  
					md5last = md5sum[31]; //stores the last byte of the sum 
					printf("Last byte of md5sum is %c\n", md5last); 
					if(md5last == '0'|| md5last == '1'|| md5last == '2'|| md5last == '3'|| md5last == '4'|| md5last == '5'|| md5last == '6'|| md5last == '7'|| md5last == '8'|| md5last == '9' )
					{
						servernumber = atoi(&md5last); 
					}
					else 
					{
						sprintf(&md5, "%x", md5last); 
						servernumber = atoi(&md5) - 51; //ascii conversion 
					}
					servernumber %= 4; 
					printf("Sending Pattern Number Adheres to (Refer table): %d\n", servernumber);	
					fclose(fptr);	
					if ( (fp = fopen(file, "rb")) == NULL)
					{
						printf("Error: File handler corrupted\n"); 
					} 
					else //calculates all the chunk sizes in which the file is to be divided
					{
						fullfilesize = findFileSize(fp);
						fclose(fp); 
						partfilesize = (fullfilesize/4); 
						lastchunksize = (fullfilesize - (3*partfilesize));						
      			} 
					char chunk[partfilesize]; 
					memset(chunk, '\0', sizeof(chunk));
					char finalchunk[lastchunksize]; 
					memset(finalchunk, '\0', sizeof(finalchunk));  
					char filepartone[partfilesize]; 
					memset(filepartone, '\0', sizeof(filepartone)); 
					char fileparttwo[partfilesize];
					memset(fileparttwo, '\0', sizeof(fileparttwo));
					char filepartthree[partfilesize];
					memset(filepartthree, '\0', sizeof(filepartthree)); 
					char filepartfour[lastchunksize];
					memset(filepartfour, '\0', sizeof(filepartfour));
					


					printf("Chunk Size = %d\n", partfilesize); 
					printf("Last Chunk Size = %d\n", lastchunksize); 
					if((fp = fopen(file, "rb")) != NULL)
					{//file open	
						for(int server = 0; server < MAX_CONNECTIONS; server++)
						{
							if(activeservers[server] == 1)
							{
								continue; 
							}	
							int k = 0;
							chunkone = distributiontable[servernumber][server]/10;  //gets second chunk number from fixed row moving coloumn
							/*deciding while chunk and size is sent to "server"*/										
							if(chunkone == 1)//sending chunk one 
							{
								fseek(fp, 0, SEEK_SET); //pointer points to the beginning of the file 
								memset(filepartone, '\0', sizeof(filepartone)); 
								fread(filepartone, partfilesize, 1, fp); //puts relevant file into that particular chunk size 
								rewind(fp); 	
								/*ENCRYPTION*/								
								k = 0;								
								for (int o = 0; o<partfilesize; o++)
								{
									chunk[o] = filepartone[o] ^ authenticate.password[k]; 
									if ( k == (strlen(authenticate.password) - 1) )
									{
										k = 0; 
									}
									else
									{
										k++; 
									}
								}
								error_t ret = sendToDFS(server, chunk, partfilesize, chunkone); 
								memset(chunk, '\0', sizeof(chunk)); 	
								memset(filepartone, '\0', sizeof(filepartone)); 						
							}
							else if(chunkone == 2)//sending chunk one 
							{
								fseek(fp, partfilesize, SEEK_SET); //pointer points to the beginning of the file 
								memset(fileparttwo, '\0', sizeof(fileparttwo));
								fread(fileparttwo, partfilesize, 1, fp); //puts relevant file into that particular chunk size
								rewind(fp);  
								k = 0;								
								for (int o = 0; o<partfilesize; o++)
								{
									chunk[o] = fileparttwo[o] ^ authenticate.password[k]; 
									if ( k == (strlen(authenticate.password) - 1) )
									{
										k = 0; 
									}
									else
									{
										k++; 
									}
								}								
								error_t ret = sendToDFS(server, chunk, partfilesize, chunkone); 								
								memset(chunk, '\0', sizeof(chunk)); 	
								memset(fileparttwo, '\0', sizeof(fileparttwo));					
							}
							else if(chunkone == 3)//sending chunk one 
							{
								fseek(fp, 2*partfilesize, SEEK_SET); //pointer points to the beginning of the file 
								memset(filepartthree, '\0', sizeof(filepartthree)); 
								fread(filepartthree, partfilesize, 1, fp); //puts relevant file into that particular chunk size 
								rewind(fp); 
								k = 0;								
								for (int o = 0; o<partfilesize; o++)
								{
									chunk[o] = filepartthree[o] ^ authenticate.password[k]; 
									if ( k == (strlen(authenticate.password) - 1) )
									{
										k = 0; 
									}
									else
									{
										k++; 
									}
								}	
								error_t ret = sendToDFS(server, chunk, partfilesize, chunkone); 								
								memset(chunk, '\0', sizeof(chunk)); 	
								memset(filepartthree, '\0', sizeof(filepartthree));	
								 								
							}
							else if(chunkone == 4)//sending chunk one 
							{
								fseek(fp, 3*partfilesize, SEEK_SET); //pointer points to the beginning of the file 
								memset(filepartfour, '\0', sizeof(filepartfour));
								fread(filepartfour, lastchunksize, 1, fp); //puts relevant file into that particular chunk size 
								rewind(fp); 
								k = 0;
								memset(finalchunk, '\0', sizeof(finalchunk)); 									
								for (int o = 0; o<lastchunksize; o++)
								{
									finalchunk[o] = (filepartfour[o] ^ authenticate.password[k]); 
									if ( k == (strlen(authenticate.password) - 1) )
									{
										k = 0; 
									}
									else
									{
										k++; 
									}
								}	
								error_t ret = sendToDFS(server, finalchunk, lastchunksize, chunkone); 															
								memset(finalchunk, '\0', sizeof(finalchunk)); 	
								memset(filepartfour, '\0', sizeof(filepartfour)); 								
							}
										
							chunktwo = distributiontable[servernumber][server]%10;  //gets second chunk number from fixed row moving coloumn
							/*deciding while chunk and size is sent to "server"*/										
							if(chunktwo == 1)//sending chunk one 
							{
								fseek(fp, 0, SEEK_SET); //pointer points to the beginning of the file
								memset(filepartone, '\0', sizeof(filepartone)); 
								fread(filepartone, partfilesize, 1, fp); //puts relevant file into that particular chunk size 
								rewind(fp); 								
								k = 0;								
								/*ENCRYPTION*/								
								for (int o = 0; o<partfilesize; o++)
								{
									chunk[o] = filepartone[o] ^ authenticate.password[k]; 
									if ( k == (strlen(authenticate.password) - 1) )
									{
										k = 0; 
									}
									else
									{
										k++; 
									}
								}
								error_t ret = sendToDFS(server, chunk, partfilesize, chunktwo); 
								memset(chunk, '\0', sizeof(chunk)); 	
								memset(filepartone, '\0', sizeof(filepartone)); 									
							}
							else if(chunktwo == 2)//sending chunk one 
							{
								fseek(fp, partfilesize, SEEK_SET); //pointer points to the beginning of the file 
								memset(fileparttwo, '\0', sizeof(fileparttwo));
								fread(fileparttwo, partfilesize, 1, fp); //puts relevant file into that particular chunk size
								rewind(fp);  	
								k = 0;								
								for (int o = 0; o<partfilesize; o++)
								{
									chunk[o] = fileparttwo[o] ^ authenticate.password[k]; 
									if ( k == (strlen(authenticate.password) - 1) )
									{
										k = 0; 
									}
									else
									{
										k++; 
									}
								}								
								error_t ret = sendToDFS(server, chunk, partfilesize, chunktwo); 								
								memset(chunk, '\0', sizeof(chunk)); 	
								memset(fileparttwo, '\0', sizeof(fileparttwo));									
							}
							else if(chunktwo == 3)//sending chunk one 
							{
								fseek(fp, 2*partfilesize, SEEK_SET); //pointer points to the beginning of the file 
								memset(filepartthree, '\0', sizeof(filepartthree));
								fread(filepartthree, partfilesize, 1, fp); //puts relevant file into that particular chunk size 
								k = 0;	
								for (int o = 0; o<partfilesize; o++)
								{
									chunk[o] = filepartthree[o] ^ authenticate.password[k];
									if ( k == (strlen(authenticate.password) - 1) )
									{
										k = 0; 
									}
									else
									{
										k++; 
									}
								}								
								error_t ret = sendToDFS(server, chunk, partfilesize, chunktwo); 								
								memset(chunk, '\0', sizeof(chunk)); 	
								memset(filepartthree, '\0', sizeof(filepartthree));								
							}
							else if(chunktwo == 4)//sending chunk one 
							{
								fseek(fp, 3*partfilesize, SEEK_SET); //pointer points to the beginning of the file 
							   memset(filepartfour, '\0', sizeof(filepartfour));
								fread(filepartfour, lastchunksize, 1, fp); //puts relevant file into that particular chunk size 
								rewind(fp);
								k = 0;
                        memset(finalchunk, '\0', sizeof(finalchunk)); 		
								for (int o = 0; o<lastchunksize; o++)
								{
									finalchunk[o] = (filepartfour[o] ^ authenticate.password[k]); 
									if ( k == (strlen(authenticate.password) - 1) )
									{
												
										k = 0; 
									}
									else
									{
										k++; 
									}
								}		
								error_t ret = sendToDFS(server, finalchunk, lastchunksize, chunktwo); 								
								memset(finalchunk, '\0', sizeof(finalchunk)); 	
								memset(filepartfour, '\0', sizeof(filepartfour));							
							}	
							memset(chunk, '\0', sizeof(chunk)); 
							memset(finalchunk, '\0', sizeof(finalchunk));
						}//for loop for all servers closes
						fclose(fp);					
						memset(file,'\0', sizeof(file));
					}//file opened	
					else 
					{
						printf("Error: File Failed to Open\n"); 
						continue; 
					}		
					
				}//else for valid out file ends
			}//put ends 

/*GET ---------------------------------------------------------------------------------------------------------*/
			else if (strcmp("get", command) == 0)
     		{//get command
				uint8_t finishedwritinglist[4] = {0}; 
				uint8_t listofchunks[4] = {0}; 
				char newfile[100] = "copied";
				strncat(newfile, file, strlen(file));

				char filepartone[1000000]; 
				memset(filepartone, '\0', sizeof(filepartone));
				char fileparttwo[1000000]; 
				memset(fileparttwo, '\0', sizeof(fileparttwo)); 
				char filepartthree[1000000]; 
				memset(filepartthree, '\0', sizeof(filepartthree)); 
				char filepartfour[1000000]; 
				memset(filepartfour, '\0', sizeof(filepartfour));
				int setsize = 0;
				int lastsize = 0; 
				//strncat(newfile, "copied", strlen("copied"));  
				for( int  u = 0; u< MAX_CONNECTIONS; u++)
				{
					if(activeservers[u] == 1)
							{
								continue; 
							}						
					/*sends command name*/ 
					noofbytesent = send(socketfiledescriptor[u], command, sizeof(command), MSG_NOSIGNAL);	
					if (noofbytesent < 0)	
	  				{
						error("Error: Writing to server\n"); 
  	  				}
					noofbytesent = send(socketfiledescriptor[u], file, sizeof(file), 0);
					if (noofbytesent < 0)	
	  				{
						error("Error: Writing to server\n"); 
  	  				}
					/*sends usersubdirectory*/
					noofbytesent = send(socketfiledescriptor[u], usersubdirectory, sizeof(usersubdirectory), MSG_NOSIGNAL);
					if (noofbytesent < 0)	
	  				{
						error("Error: Writing to server\n"); 
  	  				}

					/* sends server number*/
				
					char servernu[5]; 
					sprintf(servernu, "%d", u); 
					printf("Sending to Server Number: %s\n", servernu);
					int noofbytessent = send(socketfiledescriptor[u], servernu, sizeof(servernu), MSG_NOSIGNAL);	
					if (noofbytessent < 0)	
					{
						error("Error: Writing to server\n"); 
					} 
					int iteration = 0;
					char chunknumber[5]; 
					char chunksize[5]; 
					while(iteration < 2)				
					{//iteration loop 
						/*receive chunk number*/
						memset(chunknumber, '\0', sizeof(chunknumber)); 
						int noofbytesreceived = recv(socketfiledescriptor[u], chunknumber, sizeof(chunknumber), 0); 
						int chunknu = atoi(chunknumber); 
						printf("Chunk Number: %d\n", chunknu);
						//printf("Number of chunks received for chunk %d is %d\n", chunknu, listofchunks[chunknu-1]);					
						/*OPTIMIZATION: */
						if (chunknu > 0 && listofchunks[chunknu-1] ==  0)
						{
							//printf("In yes\n"); 
							noofbytessent = send(socketfiledescriptor[u], allow, 3, 0);
							printf("Sending Acknowledgement: %s\n", allow); 
							if (noofbytessent<0)
							{
								error(" Error: Write to client Failed\n"); 
							}	
						}
						else if (chunknu == 0 || (chunknu>0 && listofchunks[chunknu-1] !=  0))
						{//printf("In no\n"); 
							noofbytessent = send(socketfiledescriptor[u], stop, 2, 0);
							printf("Sending Acknowledgement: %s\n", stop); 
							if (noofbytessent<0)
							{
							error(" Error: Write to client Failed\n"); 
							}
							iteration++;	
							break; 
						}
						if(chunknu == 1)
						{
							if(listofchunks[chunknu-1]<2)
							{
								listofchunks[chunknu-1]++; 
							}
						}
						else if(chunknu == 2)
						{
							if(listofchunks[chunknu-1]<2)
							{
								listofchunks[chunknu-1]++; 
							}
						}
						else if(chunknu == 3)
						{
							if(listofchunks[chunknu-1]<2)
							{
								listofchunks[chunknu-1]++; 
							}
						}
						else if(chunknu == 4)
						{
							if(listofchunks[chunknu-1]<2)
							{
								listofchunks[chunknu-1]++; 
							}
						}
			
						uint8_t writechunk = 0;
						int k = 0;
						if (chunknu > 0 && listofchunks[chunknu-1] == 1)	
						{	
							/* receive chunk size*/
							memset(chunksize, '\0', sizeof(chunksize));
							noofbytesreceived = recv(socketfiledescriptor[u], chunksize, sizeof(chunksize), 0); 
							int chunksi = atoi(chunksize); 
							//printf("Chunk Size: %d\n", chunksi); 	
		
							/*receive chunk*/
							char recpacket[chunksi]; 
							memset(recpacket, '\0', chunksi); 
							noofbytesreceived = recv(socketfiledescriptor[u], recpacket, chunksi, 0); 
						//	recpacket[chunksi] = '\0';
							//printf("Chunk: %s, rec: %d\n", recpacket, noofbytesreceived); 
							
							/*	writing to file*/
						
							if (chunknu == 1)
							{
								if (chunknu > 0 && finishedwritinglist[chunknu-1] == 0)
								{
									printf("Copying first part\n");
									setsize = chunksi;
									/*decrypt chunk*/
									k = 0;
									for (int o = 0; o < chunksi; o++)
									{
										filepartone[o] = recpacket[o] ^ authenticate.password[k]; 
										if ( k == (strlen(authenticate.password) - 1) )
										{
											k = 0; 
										}
										else
										{
											k++; 
										}
									}						
									//filepartone[chunksi] = '\0'; 
									memset(recpacket, '\0', sizeof(recpacket)); 								
									//printf("Part One: %s\n", filepartone);  
									finishedwritinglist[chunknu-1] = 1;	
							 	} 
							}	
							if (chunknu == 2)
							{
								if(chunknu>0 && finishedwritinglist[chunknu-1] == 0)
								{
									printf("Copying second part\n");
									/*decrypt chunk*/
									k = 0;
									for (int o = 0; o < chunksi; o++)
									{
										fileparttwo[o] = recpacket[o] ^ authenticate.password[k]; 
										if ( k == (strlen(authenticate.password) - 1) )
										{
											k = 0; 
										}
										else
										{
											k++; 
										}
									}						
								   //fileparttwo[chunksi] = '\0'; 									
									memset(recpacket, '\0', sizeof(recpacket)); 
									finishedwritinglist[chunknu-1] = 1;	
									//printf("Part Two: %s\n", fileparttwo); 
								}
							}
						 
							if (chunknu == 3)
							{
								if (chunknu > 0 && finishedwritinglist[chunknu-1] == 0)
								{
									printf("Copying third part\n");
								   /*decrypt chunk*/
									k = 0;
									for (int o = 0; o < chunksi; o++)
									{
										filepartthree[o] = recpacket[o] ^ authenticate.password[k]; 
										if ( k == (strlen(authenticate.password) - 1) )
										{
											k = 0; 
										}
										else
										{
											k++; 
										}
									}						
									finishedwritinglist[chunknu-1] = 1;
									memset(recpacket, '\0', sizeof(recpacket)); 
								//	filepartthree[chunksi] = '\0'; 
									//printf("Part Three: %s\n", filepartthree); 
								} 
							}	
							if (chunknu == 4)
							{
								if(chunknu>0 && finishedwritinglist[chunknu-1] == 0)
								{
									printf("Copying fourth part\n");
									lastsize = chunksi; 
                        	/*decrypt chunk*/
									k = 0;
									for (int o = 0; o < lastsize; o++)
									{
										filepartfour[o] = recpacket[o] ^ authenticate.password[k]; 
										if ( k == (strlen(authenticate.password) - 1) )
										{
											k = 0; 
										}
										else
										{
											k++; 
										}
									}						
									
								  	memset(recpacket, '\0', sizeof(recpacket)); 
									finishedwritinglist[chunknu-1] = 1;
							//		printf("Fourth Part: %s\n", filepartfour); 
							 
								}
							}
							memset(recpacket, '\0', sizeof(recpacket));
						}
					  
						/*clearing all buffers*/			
						memset(chunknumber, '\0', sizeof(chunknumber)); 
						memset(chunksize, '\0', sizeof(chunksize));
					 
						iteration++; 
					} //while loop iteration ends 
				}//for loop ends 
				int zerocount = 0; 
				int onecount = 0; 
				for(int i = 0; i<4; i++)
				{
					if(listofchunks[i] == 0)
					{
						zerocount++; 
					}
					else if (listofchunks[i] == 1 && finishedwritinglist[i] == 1)
					{
						onecount++; 
					}
					printf("Chunk N0: %d Count:%d\n", (i+1), listofchunks[i]); 
				}
				if(zerocount == 4)
				{
					printf("Error: File not available. Check file name or directory\n");
				} 
				if(onecount == 4)
				{
					FILE* fp = fopen(newfile, "wb+");
					fwrite(filepartone, 1, setsize, fp);
					fwrite(fileparttwo, 1, setsize, fp);
					fwrite(filepartthree, 1, setsize, fp);
					fwrite(filepartfour, 1, lastsize, fp);
					fclose(fp);
					printf("Progress: File Complete and Available\n");
				} 
				else 
				{
					printf("Error: File Incomplete. Try again later\n"); 
				}
				memset(file, '\0', sizeof(file));
				memset(filepartone, '\0', sizeof(filepartone));
				memset(fileparttwo, '\0', sizeof(fileparttwo));
				memset(filepartthree, '\0', sizeof(filepartthree));
				memset(filepartfour, '\0', sizeof(filepartfour));
			}//get command


/*----------------LIST ------------------------*/ 
			else if (strcmp(command, "list") == 0)
			{//list begins
				char cli[500]={};
				char filestored[30][300] = {}; 
			//	char list_files[4][400] = {};
				char filesrec[4][400] = {};  
				char recbuffer[500]={}; 
				FILE *fp = fopen("tempfilelist", "w");
				printf("Progress: in list command\n"); 
				printf("Using Directory: %s\n", file);
				/*sending command and directory*/ 
				for( int  u = 0; u< MAX_CONNECTIONS; u++)
				{//for loop
					
					if(activeservers[u] == 1)
					{
						continue; 
					}
						 
					/*sends command name*/ 
					noofbytesent = send(socketfiledescriptor[u], command, sizeof(command), MSG_NOSIGNAL);	
					if (noofbytesent < 0)	
	  				{
						error("Error: Writing to server\n"); 
  	  				}
					
					/*sending server number to servers*/
					char servernu[5];
					memset(servernu, '\0', sizeof(servernu)); 
					sprintf(servernu, "%d", u); 
					printf("Sending to Server Number: %s\n", servernu);
					int noofbytessent = send(socketfiledescriptor[u], servernu, sizeof(servernu), MSG_NOSIGNAL);	
					if (noofbytessent < 0)	
					{
						error("Error: Writing to server\n"); 
					} 
					/*sends subdirectory*/
					noofbytesent = send(socketfiledescriptor[u], file, sizeof(file), MSG_NOSIGNAL);
					if (noofbytesent < 0)	
	  				{
						error("Error: Writing to server\n"); 
  	  				}
					
					/*wait for ack or nack to determine whether files are present or not*/
					char rec[3];
					memset(rec, '\0', sizeof(rec)); 
					int noofbytesreceived = recv(socketfiledescriptor[u], rec, 3, 0); 
				//	printf("Acknowledgement Received: %s of bytes: %d\n", rec, noofbytesreceived);
					if (strncmp("no", rec, 2) == 0)
					{
						printf("Error: No files in directory or wrong directory. Try Again\n");
						continue;  
					}
			
					else if(strncmp("yes", rec, 3) == 0)
					{
						printf("Progress: Getting file inventory\n");
			
						/*receiving list of files in subfolder*/
						/*Process: receive size to receive, make array, actually receive it*/ 
						 
						char filesize[5]; 
						memset(filesize, '\0', sizeof(filesize));
						noofbytesreceived = recv(socketfiledescriptor[u], filesize, sizeof(filesize), 0); 
						int size = atoi(filesize); 
						printf("Receiving File Size: %d in bytes %d\n", size, noofbytesreceived); 					
						
				
						/*receiving file containing list of files*/ 
						int recbytes = 0;
						int index = 0;
						int remainingbytes = size; 
						while(remainingbytes)
						{
							memset(recbuffer, '\0', sizeof(recbuffer));  
							recbytes = recv(socketfiledescriptor[u], recbuffer, 1, 0);
							remainingbytes -= recbytes; 
							filesrec[u][index] = recbuffer[0];
							index++;
						}
						printf("Progress: received file with  list \n"); 
											
						fwrite(filesrec[u], 1, strlen(filesrec[u]), fp); 
					}
									
				}// for loop sending inital values
				
				char endfile[] = "\n..zzz.zzz.z"; 
				fwrite(endfile, 1, sizeof(endfile), fp); 			
				fclose(fp);
				memset(recbuffer, '\0', sizeof(recbuffer)); 
					
				/*writing a cli command to sort the received file*/
				strcpy(cli, "sort tempfilelist | uniq > finalfilelist");
				system(cli); 
				memset(cli, '\0', sizeof(cli));  	
				strcpy(cli, "rm tempfilelist"); 
				system(cli); 

				fp = fopen("finalfilelist", "r"); 
				char onefile[200]; 
				memset(onefile, '\0', sizeof(onefile)); 
				int a = 0; 
				int b = 0; 
				int iteration = 0; 
				printf("List of files in directory %s:\n", file); 
				while(!feof(fp))
				{
					//printf("In while loop\n"); 
					iteration++; 
					char section[200]; 
					memset(recbuffer, '\0', sizeof(recbuffer)); 
					memset(section, '\0', sizeof(section));
					memset(onefile, '\0', sizeof(onefile)); 
					fgets(recbuffer, sizeof(recbuffer), fp); 
					strcpy(section, recbuffer); 
					if (section[0] == '.')
					{
						char storetoken[50]; 
						char *temptoken = strtok((char*)&section[1], "."); 
						strcpy(storetoken, temptoken); 
						temptoken = strtok(NULL, ".");
						
						if ((strncmp(temptoken, "1", 1) != 0) && (strncmp(temptoken, "2", 1) != 0) && (strncmp(temptoken, "3", 1) != 0) && (strncmp(temptoken, "4", 1) != 0) )				
						{
							strcat(storetoken, ".");
							strcat(storetoken, temptoken);
						} 
						sprintf(onefile, "%s", storetoken);
						//printf("%s\n", onefile);  
						if ( b == 0)
						{
							memset(filesrec[b], '\0', sizeof(filesrec[b])); 
							strncpy(filesrec[b], onefile, strlen(onefile));
							b++;
						}
						else if(strncmp(onefile, filesrec[b-1], strlen(onefile)) == 0)
						{
							a++; 
							if ( a== 3)
							{
								printf("\n%s: Complete \n", filesrec[b-1]);
								a = 0; 
							}
						}
						else 
						{
							if(a!=0)
							{
								printf("\n%s : Incomplete\n", filesrec[b-1]); 
							}
							memset(filesrec[b], '\0', sizeof(filesrec[b])); 
							strncpy(filesrec[b], onefile, strlen(onefile)); 
							b++; 
							a = 0; 
						}
					}// section 
				}// feof loop
				fclose(fp); 
				memset(cli, '\0', sizeof(cli)); 
				strcpy(cli, "rm finalfilelist"); 
				system(cli); 					
			}//list endds
/*-------------------------------EXIT-----------------------------*/
			else if (strcmp(command, "exit") == 0)
			{//exit begins 
				for( int  u = 0; u< MAX_CONNECTIONS; u++)
				{//for loop
					
					if(activeservers[u] == 1)
					{
						continue; 
					}
						 
					/*sends command name*/ 
					noofbytesent = send(socketfiledescriptor[u], command, sizeof(command), MSG_NOSIGNAL);	
					if (noofbytesent < 0)	
	  				{
						error("Error: Writing to server\n"); 
  	  				}
					printf("Closing connection to Server: %d\n", u+1); 
					close(socketfiledescriptor[u]); 
					if (returnvalue>0)
					{
						printf("Closed Client Socket\n");
					}		
				}//for loop
				exit(1);
			}//exit ends
				
		}//authenticate 
		else 
		{//not authenticate 
			printf("Error: Invalid username or password. Try Again.\n"); 
 			continue; 
		}//not authenticate 
	}//while loop ends 
	return 0;
}//main ends 
