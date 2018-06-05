/*Date: 17 November 2017*/
/****************************
 * File: Distributed File System Server
 * Subject: Network Systems 
 * Author: Arundhathi Swami 
 * Date: November 6 2017 
 * *************************/ 
#include <stdio.h>  //c library 
#include <sys/types.h> //data types used in system calls 
#include <sys/socket.h>  //definitions of structures used in sockets
#include <netinet/in.h> //constants and structures needed for internet domain addresses
#include <arpa/inet.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/aes.h>
#include <sys/dir.h>



#define BUFFER_SIZE 512
#define MAX_COMMAND_BUFFER_SIZE 500
#define MAX_FILE_NAME_BUFFER_SIZE 500

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

typedef struct credentials{
	char username[10]; 
   char password[10]; 
} cred_t;

int countLinesInFile(FILE *fp)
{
  fp = fopen("dfs.conf", "r"); 
  if (fp == NULL)
  {
    printf("Stupid File\n"); 
    return 0;
  }
  char here[2]; 
  int nol = 0;
  while(fgets(here, 2, fp))
  {
    if (here[0] == '\n')
    {
      nol++;
    }
  }
    fclose(fp);
    return nol;  
}


error_t parseConfFile(cred_t authorize[10], char file[])
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
    int lines = countLinesInFile(fp);
    int it = 0; 
   while(it < lines)
	{
    while(fgets(parsesegment, sizeof(parsesegment), fp))
    {  
      word = strtok(parsesegment, "\n");
		word = strtok(parsesegment, "\n ");
      strcpy(authorize[it].username, word);     
      word = strtok(NULL, "\n ");
      strcpy(authorize[it].password, word);
      it++;  
      
    }
	}
}
  fclose(fp); 
  return 1; 
}

/*Function to get the presence of files in a particulr directory*/
int filecheck(const struct direct *entry)
{
	if(strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) //checking for files beginning with ./..
	return 0; 
	else 
	return 1; 
}
 
int main(int argc, char* argv[])
{

	int socketfiledescriptor, portno, clientlength, returnvalue, newsockfd;
	struct sockaddr_in server_address, client_address; //struct sockaddr_in is a structure that contains an internet address 
   cred_t authorized[10];
   cred_t received;  
   char allow[3] = "yes"; 
	char stop[2]= "no"; 
	char sizeoffile[5]; 
	char subdirectory[10];
	char servernumber[5];
	char chunksize[5]; 
	
	
	char dfsfoldername[500];
	char dfsusernamefoldername[500]; 
	char dfsusernamesubdirectoryfoldername[500]; 
	char file[MAX_FILE_NAME_BUFFER_SIZE];	//accepts file name from parser function
	memset(file, '\0', sizeof(file));
   char command[MAX_COMMAND_BUFFER_SIZE];//contains command both sent and received 
	memset(command, '\0', sizeof(command));
   char username[MAX_FILE_NAME_BUFFER_SIZE];//contains command both sent and received 
	memset(username, '\0', sizeof(username));
   char password[MAX_FILE_NAME_BUFFER_SIZE];//contains command both sent and received 
	memset(password, '\0', sizeof(password));
	

	if (argc<2) //checks for number of argument passed in 
	{
		fprintf(stderr, "ERROR: No Port Provided\n"); 
		exit(1); 
	}

	socketfiledescriptor = socket(AF_INET, SOCK_STREAM, 0); 

	if (socketfiledescriptor < 0) //checks for socket function call return value 
	{
		fprintf(stderr, "ERROR: Socket Creation Error\n"); 
		//exit(1); 
	}

	bzero((char*) &server_address, sizeof(server_address)); //seta all values in a buffer to zero with first argument being a pointer to the buffer and second being the size of the buffer 

	portno = atoi(argv[1]);
	server_address.sin_family = AF_INET; 
	server_address.sin_port = htons(portno);
	server_address.sin_addr.s_addr = INADDR_ANY;

	if(bind(socketfiledescriptor, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
	{
		fprintf(stderr, "ERROR: Binding Error Occurred\n"); 
	}
	listen(socketfiledescriptor, 5); 
	clientlength = sizeof(client_address); //takes size of the client ip address
   error_t ret = parseConfFile(authorized, "dfs.conf");
	for (int it = 0; it< 4; it++)
	{
		printf(" Username:%s\n", authorized[it].username); 
   	printf(" Password:%s\n", authorized[it].password);
	} 
  	int it = 20; 
   int found = 0; 
   int noofbytessent = 0;
	int noofchild = 0; 
	int child;   
	while(1)
	{//while loop starts
		newsockfd = accept(socketfiledescriptor, (struct sockaddr *) &client_address, &clientlength);
		if(newsockfd < 0)
		{//socket failed to create 
			printf("Error: Failed to accept socket connection.\n"); 
			exit(1); 
		}//socket failed to create 
		else 
		{//successful socket 
			child = fork();
			if (child == 0)
			{//successfully created child
				noofchild++; 
				printf("Progress: Created Child ==> %d\n", noofchild); 
				close(socketfiledescriptor); //closes old sockfd
				int noofbytesreceived = recv(newsockfd, &received, sizeof(cred_t), 0); 
     			printf("Received Username = %s\n", received.username); 
   			printf("Password = %s\n", received.password);
   			if (noofbytesreceived < 0)
   			{
					error("Error: Reading from client socket\n"); 
   			}
				/*AUthenticating Username and Password*/
      		for (it = 0; it < 4; it++)
				{
					if (strcmp(received.username, authorized[it].username) == 0)
					{
						printf("Progress: Username Found\n"); 
						found = 10; 
						break; 
					}
				}
				if (found == 10)
				{//username found
					if (strcmp(received.password, authorized[it].password) == 0)
					{//password matched
						printf("Progress: Password Matched\n"); 	
						noofbytessent = send(newsockfd, allow, sizeof(allow), 0);
					   
						if (noofbytessent<0)
						{
						error(" Error: Write to client Failed\n"); 
						} 
						/*accepts command name and file name */
						noofbytesreceived = recv(newsockfd, command, sizeof(command), 0);
						if (noofbytesreceived<0)
						{
							error(" Error: Read from client Failed\n"); 
						} 
						printf("Incoming Command: %s\n", command);
						

						/*PUT COMMAND*/ 
						if (strcmp(command, "put") == 0)
						{//put command begins
							char firstfilename[500]; 
							memset(firstfilename, '\0', sizeof(firstfilename)); 
							char secondfilename[500]; 
							memset(secondfilename, '\0', sizeof(secondfilename)); 
							/*get name of the file thats coming in*/ 
							noofbytesreceived = recv(newsockfd, file, sizeof(command), 0);
				
							if (noofbytesreceived<0)
							{
								error(" Error: Read from client Failed\n"); 
							} 
							printf("Incoming File: %s\n", file); 
							/* get subdirectory if any*/
							noofbytesreceived = recv(newsockfd, subdirectory, sizeof(subdirectory), 0);
							if (noofbytesreceived<=0)
							{
								error(" Error: Read from client Failed\n"); 
							} 
							printf("Sub Directory: %s\n", subdirectory);


							/*for the first chunk, get the server number, create its directory, user folder,  and sub folder */
							noofbytesreceived = recv(newsockfd, servernumber, sizeof(servernumber), 0);
							if (noofbytesreceived<=0)
							{
								error(" Error: Read from client Failed\n"); 
							}
							int servernu = atoi(servernumber);  
							printf("Server Number for first chunk: %d\n", servernu);

							//make server directory, uske ander user directory, uske andar subfolder directory 
							if (servernu == 0)
							{//0
								strncpy(dfsfoldername, "DFS1", sizeof("DFS1"));
								
							} //0
							else if (servernu == 1)
							{//1
								strncpy(dfsfoldername, "DFS2", sizeof("DFS2"));
								
							}//1 
							else if (servernu == 2)
							{//2
								strncpy(dfsfoldername, "DFS3", sizeof("DFS3"));
								
							}//2 
							else if (servernu == 3)
							{//3
								strncpy(dfsfoldername, "DFS4", sizeof("DFS4"));
								
							}//3 
							char mkdircommand[100] = "mkdir -p ";
							strcat(mkdircommand, dfsfoldername); 
							system(mkdircommand); //creates dfs folder 
							memset(mkdircommand, '\0', sizeof(mkdircommand)); 
							strcat(dfsfoldername, "/");
							strncpy(dfsusernamefoldername, dfsfoldername, sizeof(dfsfoldername)); 
							strcat(dfsusernamefoldername, received.username);
							strncpy(mkdircommand, "mkdir -p ", sizeof("mkdir -p ")); 
							strcat(mkdircommand, dfsusernamefoldername); 
							system(mkdircommand); //creates user name sub folder within the dfs folder 
							memset(mkdircommand, '\0', sizeof(mkdircommand)); 
							strncpy(dfsusernamesubdirectoryfoldername, dfsusernamefoldername, sizeof(dfsusernamefoldername));
							strcat(dfsusernamesubdirectoryfoldername, "/"); 
							strcat(dfsusernamesubdirectoryfoldername, subdirectory); 
							strncpy(mkdircommand, "mkdir -p ", sizeof("mkdir -p ")); 
							strcat(mkdircommand, dfsusernamesubdirectoryfoldername);
							system(mkdircommand); 
							memset(mkdircommand, '\0', sizeof(mkdircommand)); 
							printf("The path to the subdirectory: %s\n", dfsusernamesubdirectoryfoldername);
							strncpy(firstfilename, dfsusernamesubdirectoryfoldername, sizeof(dfsusernamesubdirectoryfoldername)); 
							strncpy(secondfilename, dfsusernamesubdirectoryfoldername, sizeof(dfsusernamesubdirectoryfoldername)); 
							/*receiving chunk size*/
							noofbytesreceived = recv(newsockfd, chunksize, sizeof(chunksize), 0);
							if (noofbytesreceived<=0)
							{
								error(" Error: Read from client Failed\n"); 
							}
							int chunksi = atoi(chunksize);  
							printf("Chunk Size Server is receiving: %d\n", chunksi);
				
							/*receiving chunk number*/
							char number[10];
							noofbytesreceived = recv(newsockfd, number, sizeof(number), 0);
							if (noofbytesreceived<=0)
							{
								error(" Error: Read from client Failed\n"); 
							}
							int no = atoi(number);  
							printf("Chunk Number Server is receiving: %d\n", no);
						
							/*receiving chunks*/ 

							/*creating path to file*/
							if(strlen(subdirectory)) 
							{
								strcat(firstfilename, "/.");
							}
							else
							{
								strcat(firstfilename, ".");
							} 
							strcat(firstfilename, file); 
							strcat(firstfilename, "."); 
							strcat(firstfilename, number); 

							printf("Path to chunk: %s\n", firstfilename); 
					 
							char seg[chunksi];
							memset(seg, '\0', sizeof(seg));
							FILE* new  = fopen(firstfilename, "wb"); 
							if (new == NULL)
         		 		{
								printf("Error: Couldnt Open File\n");           			 
								continue; 
          				}
							int rece = 0; 
							int num = 0;
							while(rece < chunksi)
							{
								num = recv(newsockfd, seg, chunksi, 0);
								if (num<0)
								{
									error(" Error: Read from client Failed\n"); 
								}
								else 
								{
									fwrite(seg, 1, num, new);
									rece += num; 
								}
								//printf("Received %d\n", rece); 
							}
						fclose(new);
						num = 0; 
          			memset(chunksize,'\0',sizeof(chunksize));
						memset(number,'\0',sizeof(number));
						memset(seg, '\0', sizeof(seg)); 
						memset(servernumber, '\0', sizeof(servernumber));
					
					
						/*for the second chunk, get the server number, create its directory, user folder,  and sub folder */
						noofbytesreceived = recv(newsockfd, servernumber, sizeof(servernumber), 0);
						if (noofbytesreceived<=0)
						{
							error(" Error: Read from client Failed\n"); 
						}
						servernu = atoi(servernumber);  
						printf("Server Number for second chunk: %d\n", servernu); 

						/*receiving chunk size*/ 
						noofbytesreceived = recv(newsockfd, chunksize, sizeof(chunksize), 0);
						if (noofbytesreceived<=0)
						{
							error(" Error: Read from client Failed\n"); 
						}
						chunksi = atoi(chunksize);  
						printf("Chunk Size Server is receiving: %d\n", chunksi);
			
						/*receiving chunk number*/
						noofbytesreceived = recv(newsockfd, number, sizeof(number), 0);
						if (noofbytesreceived<=0)
						{
							error(" Error: Read from client Failed\n"); 
						}
						no = atoi(number);  
						printf("Chunk Number Server is receiving: %d\n", no);
							
						/*receiving chunks*/ 
					
						/*creating path to file*/
						if(strlen(subdirectory)) 
						{
							strcat(secondfilename, "/.");
						}
						else
						{
							strcat(secondfilename, ".");
						} 
						strcat(secondfilename, file); 
						strcat(secondfilename, "."); 
						strcat(secondfilename, number); 
										
						printf("Path to chunk: %s\n", secondfilename); 
					   char seg2[chunksi];
						memset(seg2, '\0', sizeof(seg2));
						new  = fopen(secondfilename, "wb"); 
						if (new == NULL)
         	 		{
							printf("Error: Couldnt open chunk file\n");           			 
							continue; 
         	 		}
						rece = 0; 
							
						while(rece < chunksi)
						{
							num = recv(newsockfd, seg2, chunksi, 0);
							if (num<0)
							{
								error(" Error: Read from client Failed\n"); 
							}
							else 
							{
								fwrite(seg2, 1, num, new);
								rece += num; 
							}
						}
						fclose(new);
						
						memset(chunksize,'\0',sizeof(chunksize));
						memset(number,'\0',sizeof(number));
						//memset(seg, '\0', sizeof(seg)); 	
						//memset(seg2, '\0', sizeof(seg2)); 
						chunksi = 0; 
						memset(servernumber, '\0', sizeof(servernumber)); 
						memset(dfsusernamesubdirectoryfoldername,'\0',sizeof(dfsusernamesubdirectoryfoldername));
						memset(file,'\0', sizeof(file));
						memset(command,'\0',sizeof(command));
						memset(subdirectory,'\0',sizeof(subdirectory));
						memset(received.username, '\0', sizeof(received.username)); 
						memset(received.password, '\0', sizeof(received.password)); 
						memset(firstfilename, '\0', sizeof(firstfilename)); 
						memset(secondfilename, '\0', sizeof(secondfilename)); 
	
					}//put command ends

/*GET--------------------------------------------------------------------*/
					else if (strcmp(command, "get") == 0)
					{//start of get 
						printf("In get \n"); 
					
						/*get name of the file thats coming in*/ 
						noofbytesreceived = recv(newsockfd, file, sizeof(command), 0);
						if (noofbytesreceived<0)
						{
							error(" Error: Read from client Failed\n"); 
						} 
						printf("Exporting File: %s \n", file); 

						/* get subdirectory if any*/
						noofbytesreceived = recv(newsockfd, subdirectory, sizeof(subdirectory), 0);
						if (noofbytesreceived<=0)
						{
							error(" Error: Read from client Failed\n"); 
						} 
				
				
						/*create four paths for finding the file parts*/
						char filepartone[300]; 
						char size[5];
						char extension[5];  
						char r[5]; 	
						/* get server number*/
						noofbytesreceived = recv(newsockfd, servernumber, sizeof(servernumber), 0);
						if (noofbytesreceived<=0)
						{
							error(" Error: Read from client Failed\n"); 
						}
						int servernu = atoi(servernumber);  
						printf("Server Number for first chunk: %d\n", servernu);
						int foundflag = 0; 

						for ( int ext = 1; ext<5; ext++)
						{
							/*creating path for chunk one*/
							memset(filepartone, '\0', sizeof(filepartone));  
							if (servernu == 0)
							{
								strncat(filepartone, "DFS1/" , sizeof("DFS1/"));
							} 
							else if (servernu == 1)
							{
								strncat(filepartone, "DFS2/" , sizeof("DFS2/"));
							} 
							else if (servernu == 2)
							{
								strncat(filepartone, "DFS3/" , sizeof("DFS3/"));
							} 
							else if (servernu == 3)
							{
								strncat(filepartone, "DFS4/" , sizeof("DFS4/"));
							} 
							strncat(filepartone, received.username, strlen(received.username));
							strncat(filepartone, "/", sizeof("/"));  
							strncat(filepartone, subdirectory, strlen(subdirectory));
							if(strlen(subdirectory)) 
							{
								strcat(filepartone, "/.");
							}
							else
							{
								strcat(filepartone, ".");
							} 
							strncat(filepartone, file, strlen(file)); 
							
							strncat(filepartone, "." , sizeof(".")); 
							memset(extension, '\0', sizeof(extension));
							sprintf(extension, "%d", ext); 
							strncat(filepartone, extension, sizeof(extension)); 
							printf("%s\n", filepartone);
							FILE *fp = fopen(filepartone, "rb"); 
							if (fp == NULL)
							{
								printf("Error: Server doesnt have requested chunk\n"); 
							} 
							
							else 
							{
								foundflag++; 
								fseek(fp, 0, SEEK_END); //takes file pointer to end of file 
								int flen = ftell(fp); //gets no of bytes of file 
								printf("Length of File: %d\n" , flen);
								rewind(fp); //takes fp back to beginning of file
							
								/*send packet number*/
								printf("Sending Chunk Number: %s\n", extension); 
								noofbytessent = send(newsockfd, extension, sizeof(extension), 0);	
								if (noofbytessent < 0)	
								{
									error("Error: Writing to server\n"); 
								} 
								/*OPTIMIZATION* ==> check if packet number is already receieved. If yes, rec a nack. if no, rec ack. if nack, continue, else nothing*/
								memset(r, '\0', sizeof(r)); 
								int noofbytesreceived = recv(newsockfd, r, sizeof(r), 0); 
								//printf("Received Acknowledgement: %s of size %d\n", r, strlen(r));
							
								if (strncmp(allow, r, 3) == 0)
								{
										
									memset(r, '\0', sizeof(r));
									/*send packet size*/
									sprintf(size, "%d", flen); 
									noofbytessent = send(newsockfd, size, sizeof(size), 0);	
									if (noofbytessent < 0)	
									{
										error("Error: Writing to server\n"); 
									} 
									printf("Sending Chunk Size: %s\n", size); 
								
									/*send actual packet*/
									char packet[flen];
									memset(packet, '\0', sizeof(packet)); 
									fread(packet, flen, 1, fp);
									//packet[flen] = '\0'; 
								//	printf("Packet sending: %s\n", packet); 
							
									noofbytessent = send(newsockfd, packet, flen, MSG_NOSIGNAL);	
									if (noofbytessent < 0)	
									{
										error("Error: Writing to server\n"); 
									} 
									
									printf("No of bytes of packet actually sent: %d\n", noofbytessent);		
									memset(packet, '\0', sizeof(packet));
									fclose(fp); 
								}
								memset(r, '\0', sizeof(r));
							}
							if( foundflag == 2)
							{
								break; 
							}
							memset(extension, '\0', sizeof(extension));
							memset(filepartone, '\0', sizeof(filepartone));	 
							
							memset(size, '\0', sizeof(size)); 
						
							}
							//printf("Found Flag Value = %d\n", foundflag); 					
						}//end of get 	

/*-----------------------LIST---------------------------*/
						else if (strcmp(command, "list") == 0)
						{//list begins
							printf("Progress: In List command\n");
							/* get server number*/
							char serverno[5];
							memset(serverno, '\0', sizeof(serverno)); 
							int noofbytesreceived = recv(newsockfd, serverno, sizeof(serverno), 0);
							if (noofbytesreceived<=0)
							{
								error(" Error: Read from client Failed\n"); 
							}
							int servernu = atoi(serverno);  
							printf("Progress: Server Number: %d\n", servernu);
	
							/*get name of the directory*/ 
							noofbytesreceived = recv(newsockfd, subdirectory, sizeof(subdirectory), 0);
							if (noofbytesreceived<0)
							{
								error(" Error: Read from client Failed\n"); 
							} 
							printf("Searching Directoy: %s \n", subdirectory); 

							

							char filepartone[300]; 
							memset(filepartone, '\0', sizeof(filepartone)); 
							/*creating file to send to client*/
							switch(servernu)
							{
								case 0:
								strncat(filepartone, "DFS1/" , sizeof("DFS1/"));
								break;
								case 1: 
								strncat(filepartone, "DFS2/" , sizeof("DFS2/"));
								break; 
								case 2:							
								strncat(filepartone, "DFS3/" , sizeof("DFS3/"));
								break;
								case 3:
								strncat(filepartone, "DFS4/" , sizeof("DFS4/"));
								break; 
							}
							strncat(filepartone, received.username, strlen(received.username));
							strncat(filepartone, "/", sizeof("/"));  
							strncat(filepartone, subdirectory, strlen(subdirectory));
							if (strlen(subdirectory))
							{
								strncat(filepartone, "/", sizeof("/")); 
							}
							printf("Created path: %s\n", filepartone);
						
							/*Navigating to file structure*/
							int count = 0; 
							int i =0; 
							struct direct **file_structure; 
							char filesread[500]; 
							memset(filesread, '\0', sizeof(filesread)); 
							count = scandir(filepartone, &file_structure, filecheck, alphasort); 
							printf("Progress: Number of Files present in directory: %d\n", count);
							if (count <= 0)
							{
								printf("Error: No files present in this directory\n");
								/*send msg*/ 
								noofbytessent = send(newsockfd, "nos", sizeof("nos"), 0);
								printf("Sent nack of bytes: %d\n", noofbytessent);
					   		if (noofbytessent<0)
								{
									error(" Error: Write to client Failed\n"); 
								} 
								close(newsockfd); //closes the new fd 
								printf("Progress: Exiting Child ==> %d\n", noofchild); 
								exit(1); 
								//continue;
								
							}
							else 
							{
								printf("Progress: Searching for Files present in directory: %d\n", count);
								noofbytessent = send(newsockfd, allow, 3, 0);
								printf("Sent ack of bytes: %d\n", noofbytessent); 
					   		if (noofbytessent<0)
								{
									error(" Error: Write to client Failed\n"); 
								}
							}	
							sleep(1);
							
							printf("Continue hoke idhar aya\n");
							/*copying files present to a sendable format*/
							for (int i = 1; i<=count; i++)
							{	
								printf("Looking at file: %s\n", file_structure[i-1]->d_name); 
								sprintf(filesread + strlen(filesread), "%s\n", file_structure[i-1] -> d_name); 
							}
							sprintf(filesread + strlen(filesread), "\n"); 
						
							/*sending length of the file*/
							int flen = strlen(filesread); 
							char filelength[5] ; 
							memset(filelength, '\0', sizeof(filelength)); 
							sprintf(filelength, "%d", flen);
							noofbytessent = send(newsockfd, filelength, sizeof(filelength), 0);
							printf("Length of file to send: %s in bytes %d\n", filelength, noofbytessent);
					   	if (noofbytessent<0)
							{
								error(" Error: Write to client Failed\n"); 
							} 		
							sleep(1);

							/*sending actual file*/ 
							noofbytessent = send(newsockfd, filesread, strlen(filesread), 0); 
							//printf("Progress: Sent list: %s in bytes %d\n", list_files, noofbytessent);
					   	if (noofbytessent<0)
							{
								error(" Error: Write to client Failed\n"); 
							} 	
								
		
						}//list endds
/*--------------------------------------EXIT--------------------*/
						else if (strcmp(command, "exit") == 0)
						{//exit begins
							printf("\nClosing Client Connection\n"); 
							returnvalue = close(newsockfd); //destroys socket
							if (returnvalue>0)
							{
								printf("Closed Server Socket\n");
							}
							printf("Progress: Exiting Child ==> %d\n", noofchild); 
							exit(1); 		
						}//exit ends				

					}//password matched 
					else
					{
						printf("Error: Password Not Matched\n"); 
						if(send(newsockfd, &stop, sizeof(stop), 0) <0)
						{
							error(" Error: Write to client Failed\n"); 
						} 
						continue; 
					}
				}//username found
				else 
				{
					printf("Username not valid\n"); 
					if(send(newsockfd, &stop, sizeof(stop), 0) <0)
					{
						error(" Error: Write to client Failed\n"); 
					} 
					continue;
				}//username invalid
 				close(newsockfd); //closes the new fd 
				printf("Progress: Exiting Child ==> %d\n", noofchild); 
				exit(1); 
			}//successfully created socket
			else
			{
				printf("Progress: Returned to main parent\n"); 
				close(newsockfd); 
			}
			
		}//successful socket 

	}//while loop ends
	return 0; 
}//main ends  
		
