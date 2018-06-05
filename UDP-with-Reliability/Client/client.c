/*
#CODE: UDP Protocol - Client Code 
#LANGUAGE: C
#AUTHOR: Arundhathi Swami
#COURSE/PROJECT: Advanced Practical Embedded Systems
#DATE STARTED: Sept 12 2017
#LAST DATE MODIFIED: Sept 24 2017
#*/

//version 2.0 - Added Reliability and Encryption of Xor 

#include <stdio.h>  //c library 
#include <sys/types.h> //data types used in system calls 
#include <sys/socket.h>  //definitions of structures used in sockets
#include <netinet/in.h> //constants and structures needed for internet domain addresses
#include <arpa/inet.h> 
#include <netdb.h> //defines structure for "hostent"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/aes.h>

#define MAX_COMMAND_BUFFER_SIZE 200//buffer size for commands 
#define MAX_FILE_BUFFER_SIZE 1048//buffer size for file 
#define MAX_RELIABLE_FILE_BUFFER_SIZE 1049//buffer size for reliability enhanced file


//error values that can be returned from various functions
typedef enum valid_commands
{
	GETS = 0, 
	PUTS = 1, 
	DELETE = 2, 
	LS = 3, 
	EXIT = 5,
} use_command; 

typedef enum ERRORS
{
	SUCCESS = 0,
	FAILURE, 
} error_t; 

// default function to print errors 
void error(char* error_message) //prints out error related to system call 
{
	perror(error_message); 
	exit(1); 
}

//function to parse commands taken in from the command line 
error_t parseCommand(char incommand[MAX_COMMAND_BUFFER_SIZE], char filename[MAX_COMMAND_BUFFER_SIZE])
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
		else
		{ 
			strncpy(filename, ptrin, strlen(ptrin)); 		
		}
		index++; 
		
		ptrin = strtok(NULL, " ,-\n"); 
	
	}
	return SUCCESS; 
}

/*--------------------LS---------------------------------*/	
//finds out all the files in the root directory at the server end and sends them back to the receiver 
//receiver ls commmand only accepts recvfroms and writes into a new file 
error_t lsCommand(int sockfd, struct sockaddr* client_address)
{
	printf("Function: List Files from Server\n"); 
	struct timeval timeout; 
	timeout.tv_sec = 0; 
	timeout.tv_usec = 0; 	
	setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));//sets timeout for all receive from functions 
	int clientlength = sizeof(struct sockaddr_in);
	char received_reliable_packet[10]; //stores the packets received with reliability
	char new_file[] = "rootlist";
	FILE *fp; 
	fp = fopen(new_file, "w+");
	if (fp == NULL)
	{
		printf("Error: File Null\n"); 
	}
	/*receives packets to write to file*/
	int returnvalue = recvfrom(sockfd, received_reliable_packet, 10, 0, client_address, &clientlength);//receives file size of rootlist
	if (returnvalue <  0)
	{
		printf("Error: Reception Errors\n"); 
		
	}
	
	char rec_packet[*received_reliable_packet]; //packet to receive file 
	 
	//receives the file 
	returnvalue = recvfrom(sockfd, rec_packet, (*received_reliable_packet), 0, client_address, &clientlength);
	int ret = fwrite(rec_packet, 1, sizeof(rec_packet), fp);
	fclose(fp);
	if(ret < 0)
	{
		printf("Error: Writing to recipient file failed\n");
		return FAILURE;
	}
	memset(received_reliable_packet, '\0', sizeof(received_reliable_packet));
	memset(rec_packet, '\0', sizeof(rec_packet)); //clears up the buffer	
	
	return SUCCESS; 
}


/*---------------------------DEL--------------------------*/
/*On the client end, this command sends a filename to the server to delete and accepts the result of that action*/
error_t deleteCommand(int sockfd, struct sockaddr* client_address, char file_to_delete[MAX_FILE_BUFFER_SIZE])
{
	printf("Function: Delete File from Server\n"); 
	struct timeval timeout; 
	timeout.tv_sec = 0; 
	timeout.tv_usec = 0; 	
	setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));//sets timeout for all receive from functions 
	int clientlength = sizeof(struct sockaddr_in);	
	char rec_packet[9]; 
	memset(rec_packet, '\0', sizeof(rec_packet)); //clears up the buffer	
	int returnvalue = recvfrom(sockfd, rec_packet, 9, 0, client_address, &clientlength);
	if (returnvalue > 1)
	{
		printf("Progress: Result of Delete Action: %s\n", rec_packet);
		printf("File Deleted: %s\n", file_to_delete); 
		return SUCCESS;
	}
	else
	{
		printf("Error: Receiving Result of Delete Action\n"); 
		return FAILURE;
	}
	memset(rec_packet, '\0', sizeof(rec_packet)); //clears up the buffer	

	
}

/*---------------------------EXIT--------------------------*/

/*At the server end this command closes the socket connection of the server*/
error_t exitCommand(int sockfd)
{
	printf("Function: Close Socket Connection\n");
	int returnvalue = 0;
	//returnvalue = shutdown(sockfd, 2); //shuts bith reception and transmission from server socket 
	if (returnvalue<0)
	{
		printf("Failed to close server socket\n"); 
		return FAILURE; 
	}
	else 
	{
		returnvalue = close(sockfd); //destroys socket
		if (returnvalue>0)
		{
			printf("Closed Client Socket\n");
		}		
		return SUCCESS;  
	}	


}


/*------------------------------------------MAIN-------------------------------------------------*/
int main(int argc, char* argv[])
{
	struct timeval timeout; 
	timeout.tv_sec = 5; 
	timeout.tv_usec = 0; 	
	int reliability_bit = 1;
	char reliability_append[1];
		

	char ack = '1'; 
	char nack = '2'; 
	
	
	char command[MAX_COMMAND_BUFFER_SIZE];//contains command both sent and received 
	memset(command, '\0', sizeof(command));
	char confirmation[MAX_COMMAND_BUFFER_SIZE];//contains ack or nack 
	memset(confirmation, '\0', sizeof(confirmation));
	char file[MAX_COMMAND_BUFFER_SIZE];	//accepts file name from parser function
	memset(file, '\0', sizeof(file));
	char file_name[MAX_COMMAND_BUFFER_SIZE]; //actual clean file name 
	memset(file_name, '\0', sizeof(file_name));
	char send_packet[MAX_FILE_BUFFER_SIZE]; 	//stores file buffers to send
	memset(send_packet, '\0', sizeof(send_packet));
	char send_reliable_packet[MAX_RELIABLE_FILE_BUFFER_SIZE]; //stores file to send with reliability
	memset(send_reliable_packet, '\0', sizeof(send_reliable_packet));
	char received_reliable_packet[MAX_RELIABLE_FILE_BUFFER_SIZE]; //stores the packets received with reliability
	memset(received_reliable_packet, '\0', sizeof(received_reliable_packet));
	char received_packet[MAX_FILE_BUFFER_SIZE]; //stores the file to be sent back to the client or to be taken from the client 
	memset(received_packet, '\0', sizeof(received_packet));
	

	
	int socketfiledescriptor, portno, clientlength, returnvalue; 
	struct sockaddr_in server_address, client_address; //struct sockaddr_in is a structure that contains an internet address 

	struct hostent *server; //hostent is a structure that defines a host computer on the internet 

	if(argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0); 
	}
//argc checks the port number validity and existence 	
	socketfiledescriptor = socket(AF_INET, SOCK_DGRAM, 0); 	
	portno = atoi(argv[2]);
	
	if (socketfiledescriptor<0)
	{
		error("ERROR: Could not open socket\n"); 
	} //checks for initialization of socket on client end 
	
	server_address.sin_family = AF_INET;  //assigns af_inet as the address family 
	server = gethostbyname(argv[1]); 

	if (server == NULL)
	{
		fprintf(stderr, "ERROR: No such host found\n"); 
		exit(0);
	}

	bcopy((char*)server->h_addr, (char*)&server_address.sin_addr, server->h_length); 
	server_address.sin_port = htons(portno); //assigns port number to appropriate structure member converting it from host name to network resolvable name 
	
	clientlength = sizeof(struct sockaddr_in);

	/*----------------------------sets timeout value-------------------------------------------------------------------------*/
	 	
	setsockopt (socketfiledescriptor, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));//sets timeout for all receive from functions 
	
	/*---------------------------------------------------while loop-----------------------------------------------------------------------------------------------*/
	while (1)	
	{
		//section to send command to server 	
		printf("\n\nPlease enter one of five options: \nOption 1: get [filename] \nOption 2: put [filename] \nOption 3: delete [filename] \nOption 4: Print list of files \nOption 5: exit\n"); 
		memset(command, '\0', sizeof(command));//clears command buffer 
		fgets(command, MAX_COMMAND_BUFFER_SIZE, stdin); //accepts string from command line 
		char* ptr_to_command = command;
		char* ptr_to_commands;

		//sending command 
		returnvalue = sendto(socketfiledescriptor, command, strlen(command), 0, (struct sockaddr*)&server_address, clientlength); //sends string to server 
		if(returnvalue < 0)
		{
			error("Error: Writing to socket failed"); 
		}
		
		//waits until it receives ack or nack 
		memset(confirmation, '\0', sizeof(confirmation));
		while (returnvalue = recvfrom(socketfiledescriptor, confirmation, MAX_COMMAND_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &clientlength) <= 0)
		{
				continue; 
		}
		//printf("\nreturn value for cnf: %d\n",returnvalue);	
	
		printf("Command Received Status: %s\n", confirmation);
		if (*confirmation == ack)
		{
			memset(confirmation, '\0', sizeof(confirmation));//clear  confirmation buffer 
			int ret = parseCommand(ptr_to_command, file);		
			printf("\nCommand: %s\n", command);
			printf("File: %s\n", file);
			memset(confirmation, '\0', sizeof(confirmation));
		}
		else 
		{
			printf("\n%s\n", "Error:Not Acknowledged, Resend");
			memset(confirmation, '\0', sizeof(confirmation));//clear  confirmation buffer 
		}
	
		//this completes initial command exchange between server and client
	
		memset(confirmation, '\0', sizeof(confirmation));//clear  confirmation buffer 
		//end of function calling 
/*--------------------------------------------------------------------------calling different commands---------------------------------------------------------*/
	
/*---------------------------GET--------------------------*/
		if (strcmp("get", command) == 0)
		{//beginning of get command 	
			printf("Function: Receiving File from Remote Server\n");
			int incoming_file_length = 0;
			returnvalue = recvfrom(socketfiledescriptor, &incoming_file_length, sizeof(int), 0, (struct sockaddr*)&client_address, &clientlength); //receives file length
			if (incoming_file_length == 0)
			{ //file validity check 
				printf("Error: Invalid File Name. Exiting Command.\n"); //exits file 
				memset(command, '\0', sizeof(command));//clearing command buffer 
				memset(file, '\0', sizeof(file));//clearing file buffer 		 
			}//end of file validity check 
			else 
			{//received valid file 
				printf("Incoming File Length: %d\n", incoming_file_length);
				memset(command, '\0', sizeof(command));//clearing command buffer 
				returnvalue = sendto(socketfiledescriptor, &ack, sizeof(char), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);//send ack for having received file length
				int last_packet_size = 0; 
				int final_buffer_transfers = 0;
				int no_of_buffer_transfers = (incoming_file_length/MAX_FILE_BUFFER_SIZE); 					
				last_packet_size = (incoming_file_length%MAX_FILE_BUFFER_SIZE);
				if (last_packet_size)
				{//calculates total no of transfers 
					int final_buffer_transfers = (no_of_buffer_transfers+1); 
					printf("Number of Buffer Transfers Expected: %d\n", (final_buffer_transfers)); 
				}//end of calculates total no of transfers 
				else
				{//else calculates total no of transfers 
					int final_buffer_transfers = no_of_buffer_transfers;
					printf("Number of Buffer Transfers Expected: %d\n", (final_buffer_transfers)); 
				}//end of else calculates total no of transfers 
				char new_file[] = "copied";
				strcat(new_file,file); //names the new file
				
				//printf("File Name Length: %d\n", strlen(new_file));
				memset(file, '\0', sizeof(file));//clearing file buffer 					
					
				FILE *fp; 
				fp = fopen(new_file, "w+"); 
				//starts receiving bufffers for file
				int iterations = 0;
				int iteration_no = 0; 
				printf("Progress: Receiving File Commenced\n");
/*****************---------------------------Reliability Implementation starts here----------------------------------------*****************************/
				for ( iterations=0; iterations<(no_of_buffer_transfers); iterations++)
				{//loop to receive transmissions 
					int wrong_packet_counter = 0; 	
					while(1)
					{//reliable loop
						//receives single packet 						
						returnvalue = recvfrom(socketfiledescriptor, received_reliable_packet, MAX_RELIABLE_FILE_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &clientlength);
						
						if (returnvalue >= 0)
						{//valid packet received 
							if ((received_reliable_packet[0]-'0') == reliability_bit)
							{	//start of checking paacket validity
								//send acknowledgement for right packet 
								returnvalue = sendto(socketfiledescriptor, &reliability_bit, sizeof(char), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);//send ack for having received file length							
								iteration_no += 1;							
								printf("Packet Number Received: %d => Confirmation Number: %d\n", iteration_no, reliability_bit); 

								for (uint32_t j = 0; j<MAX_FILE_BUFFER_SIZE; j++)
								{
									received_packet[j] = received_reliable_packet[j+1]; 
									received_packet[j] = received_packet[j] ^ (0xFFFF);
								}
								printf("Size of Packet Received: %ld\n", sizeof(received_packet));
								//writes to the file 
								if(fwrite(received_packet, 1, MAX_FILE_BUFFER_SIZE, fp)<=0)
								{
									printf("Error: Writig to recipient file failed.\n");
								} 
								memset(received_packet, '\0', sizeof(received_packet));//clearing command buffer 
								memset(received_reliable_packet, '\0', sizeof(received_reliable_packet));//clearing command buffer	
								reliability_bit ^= 1;
								break;  
							}//end of checking packet validity
							else 
							{//invalid packet received, handling if ack is lost
								wrong_packet_counter += 1; 
								printf("Error: Wrong Packet Received\n"); 
								if (wrong_packet_counter > 1)
								{
									reliability_bit ^= 1;
									//send ack for previous bit
									returnvalue = sendto(socketfiledescriptor, &reliability_bit, sizeof(char), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);//send ack for having received file length
									reliability_bit ^= 1;
								}
							}//invalid packet received 
						}//end of valid packet received 
					}//end of while loop 
						
				}//end of for loop to receive transmissions 
				if (last_packet_size)
				{//start of loop to receive last packet 						
					char last_reliable_packet[last_packet_size+1];
					char last_packet[last_packet_size]; 
					char dec_last_reliable_packet[last_packet_size+1];
					//receiving last packet
					int wrong_packet_counter = 0; 	
					while(1)
					{//reliable transfer while loop
						returnvalue = recvfrom(socketfiledescriptor, last_reliable_packet, (last_packet_size+1), 0, (struct sockaddr*)&client_address, &clientlength);
						if (returnvalue >= 0)
						{//received packet 
							if ((last_reliable_packet[0]-'0') == reliability_bit)
							{ //start of check for right packet 
					   		//send ack for having received file 						
								returnvalue = sendto(socketfiledescriptor, &reliability_bit, sizeof(char), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);
								
								iteration_no += 1;							
								printf("Packet Number Received: %d => Confirmation Number: %d\n", iteration_no, reliability_bit); 		
	
								for (uint32_t j = 0; j<last_packet_size; j++)
								{
									last_packet[j] = last_reliable_packet[j+1]; 
									last_packet[j] = last_packet[j] ^ (0xFFFF);
								}
							
								printf("Size of Packet Received: %ld\n", sizeof(last_packet));
								if(fwrite(last_packet, 1, last_packet_size, fp)<=0)
								{
									printf("Error:Writing to recipient file failed.\n");
								} 
								memset(last_reliable_packet, '\0', sizeof(last_reliable_packet));//clearing command buffer 
								memset(last_packet, '\0', sizeof(last_packet));//clearing command buffer
								reliability_bit ^= 1;
								break; 
							}//end of checking for right packet
							else 
							{//wrong packet received 
								wrong_packet_counter += 1; 
								printf("Wrong Last Packet Received\n"); 
								if (wrong_packet_counter > 1)
								{
									reliability_bit ^= 1;
									returnvalue = sendto(socketfiledescriptor, &reliability_bit, sizeof(char), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);
									reliability_bit ^= 1;										
									//send ack for previous bit
								}
							}//end of wrong packet received
						}//end of received packet
					}//end of while loop  
				}//end of loop to receive last packet 	
				printf("Progress: Finished Receiving\nGET FILE COMMAND COMPLETED");	
				fclose(fp);	
				reliability_bit = 1; 
				//decrypt file
								
			}//end of received valid file 
			
		}//end of get command 



/*---------------------------PUT--------------------------*/
		if (strcmp("put", command) == 0)
		{//start of put command 
			
			printf("Function: Sending File To Remote Server\n");
			int reliability_bit = 1; 
			int flen = 0; 
			FILE* fp = fopen(file, "r"); 
			if (fp == NULL)
			{//non existence of file 
				printf("Error: Invalid File. Exiting Command\n");
				returnvalue = sendto(socketfiledescriptor, &flen, sizeof(int), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);//send length of file 	
				memset(command, '\0', sizeof(command));//clearing command buffer 
			}//end of non existence of file  
			else
			{//start of file exists 
				printf ("Progress: File Found.\n");
				//file is encrypted
				//printf("Length of ENcrypted file name: %d\n", strlen(file));
				FILE *encfp = fopen(file, "r");
				if (encfp == NULL)
				{
					printf("Error: Encryption Failed\n"); 
				}
				fseek(encfp, 0, SEEK_END); //takes file pointer to end of file 
				flen = ftell(encfp); //gets no of bytes of file 
				printf("Length of File: %d\n" , flen);
				rewind(encfp); //takes fp back to beginning of file 
				returnvalue = sendto(socketfiledescriptor, &flen, sizeof(int), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);//sends file length	
				memset(command, '\0', sizeof(command));//clearing command buffer
				//wait for ack for file length
				returnvalue = recvfrom(socketfiledescriptor, confirmation, MAX_COMMAND_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &clientlength); //waits to receive ack
				printf("File Length Confirmation Received: %s\n", confirmation);
				int last_packet_size = 0; 
				int final_buffer_transfers = 0;
				int no_of_buffer_transfers = (flen/MAX_FILE_BUFFER_SIZE); 					
				last_packet_size = (flen%MAX_FILE_BUFFER_SIZE);
				if (last_packet_size)
				{//calculates total no of transfers 
					int final_buffer_transfers = (no_of_buffer_transfers+1); 
					printf("Number of Buffer Transfers Required: %d\n", (final_buffer_transfers)); 
				}//end of calculates total no of transfers 
				else
				{//else calculates total no of transfers 
					int final_buffer_transfers = no_of_buffer_transfers;
					printf("Number of Buffer Transfers Required: %d\n", (final_buffer_transfers)); 
				}//end of else calculates total no of transfers  
				if (*confirmation == ack)
				{//packet length confirmed, start transmission 
					memset(confirmation, '\0', sizeof(confirmation));//clearing command buffer 
					printf("Progress: Sending File Now.\n");
					int iterations = 0;
					int number_of_iterations = 0;
/*****************---------------------------Reliability Implementation starts here----------------------------------------*****************************/
					while (iterations < no_of_buffer_transfers) //iterates over initial number of buffer transfers 
					{//starting first n iterations 
						if(fread(send_packet, 1,MAX_FILE_BUFFER_SIZE,encfp) <= 0) //bytes are read properly into the sending buffer 
						{	//if for fread 
							printf("Error: Reading bytes from file to buffer in first loop\n"); 
							break;
						}//end of if for fread 
						else 
						{//else for sending after successful fread
							sprintf(reliability_append, "%d",reliability_bit); 
							strcpy(send_reliable_packet,reliability_append); //appending reliability bit with 0 or 1				
							//creates final sendable  buffer iwth reliability bit concatenated with data							  
							for (uint32_t j = 0; j<MAX_FILE_BUFFER_SIZE; j++)
							{
								send_packet[j] = send_packet[j] ^ (0xFFFF); //xor encrypting								
								send_reliable_packet[j+1] = send_packet[j]; 
							}//final packet ready for sending 	
							
							while(1)
							{//successful transfer loop
								//send the reliability bit appended packet
								returnvalue = sendto(socketfiledescriptor, send_reliable_packet, (MAX_FILE_BUFFER_SIZE+1), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);
								//waits for ack in the form of the sent packet number 
								returnvalue = recvfrom(socketfiledescriptor, confirmation, MAX_COMMAND_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &clientlength);
								//in case of a failed packet ack is not received 
								if (returnvalue<0)
								{//packet delivery failed ack not received 
									printf("Error: Timeout Auto Detected\n"); 
									continue; 
								}//end of failed packet delivery 
								else //ack received 
								{//start of ack received 
									
									if (*confirmation == reliability_bit)
									{//start if correct packet number received
										number_of_iterations += 1;
										printf("Packet Number Sent: %d => Confirmation Number: %d\n", number_of_iterations, *confirmation); 
										memset(confirmation, '\0', sizeof(confirmation));//clearing command buffer 			
										memset(send_packet, '\0', sizeof(send_packet));//clearing command buffer 
										memset(send_reliable_packet, '\0', sizeof(send_reliable_packet));//clearing command buffer 
										reliability_bit ^= 1; 	
										iterations++;
										break; 
									}//end of correct packet received check
									else 
									{//start of wrong frame sent 
										continue; 
									}//end of wrong packet sent 
								}//end of ack recieved
							}//end of packet sending while loop 
						}//end of else after after sending 
					}//end of while to send first n iterations 
					if (last_packet_size)
					{//start of sending last packet 
						char send_last_packet[last_packet_size]; 
						char enc_send_last_reliable_packet[last_packet_size+1];	
						if(fread(send_last_packet, 1,last_packet_size,encfp) <= 0)
						{																	
							printf("Error: Reading file in last loop\n"); 
						}
						else 
						{//else for correct file read 
							char send_last_reliable_packet[last_packet_size+1]; 
							sprintf(	reliability_append, "%d",reliability_bit); 
							strcpy(send_last_reliable_packet,reliability_append); //appending reliability bit with 0 or 1	
							for (uint32_t j = 0; j<last_packet_size; j++)
							{
								send_last_packet[j] = send_last_packet[j] ^ (0xFFFF); 								
								send_last_reliable_packet[j+1] = send_last_packet[j]; 
							}
							//sends reliable packet 
							while(1)
							{//successful transfer loop
								//sends reliable packet 
								returnvalue = sendto(socketfiledescriptor, send_last_reliable_packet, (last_packet_size+1), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);
								//waits for last received ack
								returnvalue = recvfrom(socketfiledescriptor, confirmation, MAX_COMMAND_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &clientlength);
								//in case of a failed packet ack is not received 
								if (returnvalue<0)
								{//packet delivery failed ack not received 
									printf("Error: Timeout Auto Detected\n"); 
									continue; 
								}//end of failed packet delivery 
								else //ack received 
								{//start of ack received 
									if (*confirmation == reliability_bit)
									{//if for checking correct packet 
										number_of_iterations += 1;
										printf("Packet Number Sent: %d => Confirmation Number: %d\n", number_of_iterations, *confirmation); 
										printf("Finished Last Sending Packet\n");
										memset(confirmation, '\0', sizeof(confirmation));//clearing command buffer 			
										memset(send_last_packet, '\0', sizeof(send_last_packet));//clearing command buffer 
										memset(send_last_reliable_packet, '\0', sizeof(send_last_reliable_packet));//clearing command buffer 
										reliability_bit ^= 1; 
										break;	
									}//end of checking correct packet 
									else 
									{//start of wrong frame sent 
										printf("Error: Last Packet Corrupted\n"); 
										continue; 
									}//end of wrong packet sent 
								}//end of ack recieved
							}//end of packet sending while loop 
						}//end of else for coorect file read
					}//end of if loop for sending last packet
					printf("Progress: Finished Sending File\nPUT FILE COMMAND COMPLETED\n");
					fclose(encfp);
					reliability_bit = 1;
				}//end transmission

				else  //ack wasnt received, itll exit the command 
				{//start of no ack, exit 
					printf("Error: On Server Side. Exiting Command\n");
					returnvalue = sendto(socketfiledescriptor, &flen, sizeof(int), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);	//sends 0 file length 
					memset(command, '\0', sizeof(command));//clearing command buffer 
					memset(file, '\0', sizeof(file));//clearing file buffer 
				}//end of no ack exit 
			}//end of file exists 	
		memset(file, '\0', sizeof(file));//clearing file buffer 
		}//end of put command 
	
/*---------------------------DEL--------------------------*/
		if (strcmp("delete", command) == 0)
		{
			struct sockaddr* remote = (struct sockaddr*)&client_address; 
			returnvalue = deleteCommand(socketfiledescriptor, remote, file);  
			int r; 
			if(returnvalue == FAILURE)
			{
				error("Error: Action Failed"); 
			}
			else
			{
				printf("\nDELETE FILE COMPLETED\n"); 
			}

			memset(command, '\0', sizeof(command));//clearing command buffer 
			memset(file, '\0', sizeof(file));//clearing file buffer 
		}
	
/*--------------------LS---------------------------------*/	
		if (strcmp("ls", command) == 0)
		{	
			struct sockaddr* remote = (struct sockaddr*)&client_address; 
			returnvalue = lsCommand(socketfiledescriptor, remote); 
			if(returnvalue == FAILURE)
			{
				printf("Error: Action Failed"); 
			}
			else
			{
				printf("\nLIST ROOT DIRECTORY COMPLETED\n"); 
			}
			//memset(command, '\0', sizeof(command));//clearing command buffer 
		}

/*---------------------------EXIT--------------------------*/
		if (strcmp("exit", command) == 0)
		{
			returnvalue = exitCommand(socketfiledescriptor); 
			int r; 
			if(returnvalue == FAILURE)
			{
				printf("Error: Action Failed"); 
			}
			else
			{
				printf("\nEXIT CLIENT SOCKET COMPLETED\n");
				return 1;  
			}
		}
	
	} // main while loop ends 
	return 1; 
}

