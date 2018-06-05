/*
#CODE: UDP Protocol - Server Code 
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/aes.h>

#define MAX_COMMAND_BUFFER_SIZE 200 //buffer size for commands 
#define MAX_FILE_BUFFER_SIZE 1048 //buffer size for file 
#define MAX_RELIABLE_FILE_BUFFER_SIZE 1049 //buffer size for reliability enhanced file
#define VALID_COMMANDS 5


//error values that can be returned from various functions
typedef enum ERRORS
{
	SUCCESS = 0,
	FAILURE, 
	COMMAND_VALID,
	COMMAND_INVALID
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
			//printf("\nFunction Command: %s\n", store);
			
		}
		else
		{ 
			//strncpy(storein, ptrin, strlen(ptrin));
			//printf("\nFile Name: %s\n", storein);
			strncpy(filename, ptrin, strlen(ptrin));
		}
		index++; 
		ptrin = strtok(NULL, " ,-\n"); 
	
	}
	
	return SUCCESS; 
}


/*--------------------LS---------------------------------*/	
/*This command on the server end, send a list of the files present in the servers root directory to the client*/
error_t lsCommand(int sockfd, struct sockaddr* client_address)
{
	printf("Function: List Files\n"); 
	char new_file[] = "rootlist";
	FILE *fp; 
	fp = fopen(new_file, "w+");
	system("ls > rootlist"); //calls system command for enlisting files in root directory and stores them to a file called rootlist 
	FILE *fptr; 
	fptr = fopen(new_file, "r"); 
	int flen;
	if (fptr != NULL) //if file is found 
	{
		fseek(fptr, 0, SEEK_END); //takes file pointer to end of file 
		flen = ftell(fptr); //gets no of bytes of file 
		rewind(fptr); //takes fp back to beginning of file 
	}
	else 
	{
		printf("Null file\n");
	}
	struct timeval timeout; 
	timeout.tv_sec = 0; 
	timeout.tv_usec = 0; 	
	setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));//sets timeout for all receive from functions 
	int clientlength = sizeof(struct sockaddr_in);
 
	int returnvalue = sendto(sockfd, &flen, sizeof(int), 0,client_address, (socklen_t)clientlength); //sends suze of expected rootlist delivery buffer 
	
	if(returnvalue <0)//checks for successful sending
	{
		error("ERROR: Nack"); 
		return FAILURE;
	}
	else
	{
		char send_packet[flen]; //stores the packets received with reliability	
		if(fread(send_packet, 1,flen, fptr) <= 0) //bytes are read properly into the sending buffer 
		{	
			printf("Error: Reading bytes from file to buffer\n"); 
		}
		fclose(fptr);	
		returnvalue = sendto(sockfd, send_packet, flen, 0,client_address, (socklen_t)clientlength);
		if (returnvalue > 0 )
		{
			memset(send_packet, '\0', sizeof(send_packet));//clears the buffer of the file 			
			return SUCCESS;
		}
		else
		{
			memset(send_packet, '\0', sizeof(send_packet));//clears the buffer of the file 
			return FAILURE; 	
		}
			
	}
	
}


/*---------------------------DEL--------------------------*/
/*ON the server end this command receives the name of the file to be deleted and deleted it and sends a result of the action back to the client*/
error_t deleteCommand(int sockfd, struct sockaddr* client_address, char* file_to_delete)
{
	printf("Function: Delete File\n"); 
	printf("File To Delete: %s\n", file_to_delete);
	struct timeval timeout; 
	timeout.tv_sec = 0; 
	timeout.tv_usec = 0; 
	char send_packet_success[7] = "SUCCESS";	
	char send_packet_failure[7] = "FAILURE";	
	setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));//sets timeout for all receive from functions 
	int clientlength = sizeof(struct sockaddr_in);
	FILE* fp = fopen(file_to_delete, "r"); 
	int returnvalue = 0;
	if (fp == NULL)
	{
		printf("Error: File Not Found\n"); 
		returnvalue = sendto(sockfd, send_packet_failure, 7, 0,client_address, (socklen_t)clientlength);
		memset(send_packet_failure, '\0', sizeof(send_packet_failure));
		return FAILURE; 
	}
	if (remove(file_to_delete) < 0 )
	{
		printf("Error: File could not be deleted\n"); 
		returnvalue = sendto(sockfd, send_packet_failure, 7, 0,client_address, (socklen_t)clientlength);
		memset(send_packet_failure, '\0', sizeof(send_packet_failure));
		return FAILURE; 
	}
	else 
	{
		printf("Success: File Deleted\n"); 
		returnvalue = sendto(sockfd, send_packet_success, 7, 0,client_address, (socklen_t)clientlength);
		memset(send_packet_success, '\0', sizeof(send_packet_success));
		return SUCCESS; 
	}
	
}


/*---------------------------EXIT--------------------------*/
/*At the server end, this command closes the socket connection of the server*/
error_t exitCommand(int sockfd)
{
	printf("Function: Exit Spcket Connection\n"); 
	int returnvalue = 0; 
	returnvalue = shutdown(sockfd, 2); //shuts both reception and transmission from server socket 
	if (returnvalue<0)
	{
		printf("Error: Failed to shutdown Socket gracefully. Try Again\n"); 
		return FAILURE; 
	}
	else 
	{
		returnvalue = close(sockfd); //destroys socket
		if (returnvalue>0)
		{
			printf("Success:Shut down gracefully and Closed Server Socket\n");
		}		
		
		return SUCCESS;  
	}
}

/*----------------------------------------------------------------------MAIN-----------------------------*/
int main (int argc, char* argv[])
{
	int socketfiledescriptor, portno, clientlength, returnvalue;
	
	struct timeval timeout; //sets struct for timeout 
	timeout.tv_sec = 5; //timeout set at 5 secs 
	timeout.tv_usec = 0; 
	int reliability_bit = 1;
	char reliability_append[1];

/*--------initialising encryption variables--------------*/ 
	
	char ack = '1'; //defines ack value 
	char nack = '2'; //defines nack value
	
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
	

	struct sockaddr_in server_address, client_address; //struct sockaddr_in is a structure that contains an internet address 

	if (argc<2) //checks for number of argument passed in 
	{
		fprintf(stderr, "ERROR: No Port Provided\n"); 
		exit(1); 
	}

	socketfiledescriptor = socket(AF_INET, SOCK_DGRAM, 0); 

	if (socketfiledescriptor < 0) //checks for socket function call return value 
	{
		fprintf(stderr, "ERROR: Socket Creation Error\n"); 
		exit(1); 
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

	clientlength = sizeof(client_address); //takes size of the client ip address 

/*----------------------------sets timeout value-------------------------------------------------------------------------*/
	setsockopt (socketfiledescriptor, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));//sets timeout for all receive from functions 

/*-----------------------------------------------Starts Server Functioning-----------------------------------------------------*/
	
	while(1)
	{
		printf("\nWaiting for input from the client\n");
		memset(command, '\0', sizeof(command));//clearing command buffer 
		memset(file, '\0', sizeof(file));//clearing file buffer 		
		
		while(returnvalue = recvfrom(socketfiledescriptor, command, MAX_COMMAND_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &clientlength)<0)//gets command from client 
		{
				continue; 
		}
		//send an ack after having received the first command  
		returnvalue = sendto(socketfiledescriptor, &ack, sizeof(char), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);
		printf("\nReceived Datagram: %s\n", command); 

		
		/*---------------------------------Parsing command------------------------------------------------------*/		
		
		int no_of_buffer_transfers = 0;	
		char* ptr_to_command = command; 
		int ret = parseCommand(command, file); 
		printf("Command: %s\n", command);
		printf("File: %s\n", file);
		
/*---------------------------GET--------------------------*/
//At the server end, this command searches for the required file and send it to the remote clien
		if (strcmp("get", command) == 0)
		{//start of get command 
			printf("Function: Sending File To Remote Client\n");
			int reliability_bit = 1; //initialises first packet number
			int flen = 0; 
			FILE* fp = fopen(file, "r"); 
			if (fp == NULL)
			{//non existence of file 
				printf("Error: Invalid File. Exiting Command\n");
				returnvalue = sendto(socketfiledescriptor, &flen, sizeof(int), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);	//sends 0 file length 
				memset(command, '\0', sizeof(command));//clearing command buffer 
			}//end of non existence of file  
			else
			{//existence of file
				printf ("File Found.\n");
				fseek(fp, 0, SEEK_END); //takes file pointer to end of file 
				flen = ftell(fp); //gets no of bytes of file 
				printf("Length of File: %d\n" , flen);
				rewind(fp); //takes fp back to beginning of file
				FILE *encfp = fopen(file, "r");
				if (encfp == NULL)
				{
					printf("Error: File Corrupt\n"); 
				}
				else 
				{//encrypted file exists
						fseek(encfp, 0, SEEK_END); //takes file pointer to end of file 
						flen = ftell(encfp); //gets no of bytes of file 
						rewind(encfp); //takes fp back to beginning of file
						returnvalue = sendto(socketfiledescriptor, &flen, sizeof(int), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);	//send calculated file length 
						returnvalue = recvfrom(socketfiledescriptor, confirmation, MAX_COMMAND_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, &clientlength);//wait for ack for file length
						printf("File Length Confirmation Received: %s\n", confirmation);
						int last_packet_size = 0; 
						int final_buffer_transfers = 0;
						int no_of_buffer_transfers = (flen/MAX_FILE_BUFFER_SIZE); 					
						last_packet_size = (flen%MAX_FILE_BUFFER_SIZE);
						if (last_packet_size)
						{//calculates total no of transfers 
							int final_buffer_transfers = (no_of_buffer_transfers+1); 
							printf("No. of Buffer Transfers Required: %d\n", (final_buffer_transfers)); 
						}//end of calculates total no of transfers 
						else
						{//else calculates total no of transfers 
							int final_buffer_transfers = no_of_buffer_transfers;
							printf("No. of Buffer Transfers Required: %d\n", (final_buffer_transfers)); 
						}//end of else calculates total no of transfers 
						if (*confirmation == ack)//after having received an ack from client for getting file length
						{//start of file sending after receiving an ack 
							memset(confirmation, '\0', sizeof(confirmation));//clearing command buffer 
							printf("Progress: Sending File Now.\n");
							int iterations = 0;
							int number_of_iterations = 0;
/*****************---------------------------Reliability Implementation starts here----------------------------------------*****************************/
							while (iterations < no_of_buffer_transfers) //iterates over initial number of buffer transfers 
							{//starting first n iterations 
								if(fread(send_packet, 1,MAX_FILE_BUFFER_SIZE,encfp) <= 0) //bytes are read properly into the sending buffer 
								{	//if for fread 
									printf("Error: Reading bytes from file to buffer in Iterations\n"); 
									break;
								}//end of if for fread 
								else 
								{//else for sending after successful fread
									sprintf(reliability_append, "%d",reliability_bit); 
									strcpy(send_reliable_packet,reliability_append); //appending reliability bit with 0 or 1				
									//creates final sendable  buffer iwth reliability bit concatenated with data							  
									for (uint32_t j = 0; j<MAX_FILE_BUFFER_SIZE; j++)
									{
										send_packet[j] = send_packet[j] ^ (0xFFFF);										
										send_reliable_packet[j+1] = send_packet[j]; 
									}//completes building packet	
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
								}//end of bad file read else loop
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
											printf("%d\n", *confirmation); 
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
							}//end of sending last packet
							printf("Progress: Finished Sending File\nGET FILE COMMAND COMPLETED\n");
							fclose(encfp);
							reliability_bit = 1;
						}//end of file sending
				
						else  //ack wasnt received, itll exit the command 
						{//start of no ack, exit 
							printf("Error: On Client Side. Exiting Command\n");
							returnvalue = sendto(socketfiledescriptor, &flen, sizeof(int), 0,(struct sockaddr*)&client_address, (socklen_t)clientlength);	//sends 0 file length 
							memset(command, '\0', sizeof(command));//clearing command buffer 
							memset(file, '\0', sizeof(file));//clearing file buffer 
							
						}//end of no ack exit 
					} //end of encrypted file exists
				}//end of existence of file 
				memset(file, '\0', sizeof(file));//clearing file buffer 
			}//end of get command 



/*---------------------------PUT--------------------------*/
//On the server side, put function receives the file the client is sending to it
		if (strcmp("put", command) == 0)
		{//beginning of put command 
			memset(command, '\0', sizeof(command));//clearing command buffer 
			printf("Function: Receiving file from remote Client\n");
			//starts waiting for file to come in 
			int incoming_file_length = 0;
			returnvalue = recvfrom(socketfiledescriptor, &incoming_file_length, sizeof(int), 0, (struct sockaddr*)&client_address, &clientlength); //receives file length
			if (incoming_file_length == 0)
			{ //file validity check 
				printf("Error: Invalid File Name. Exiting Command.\n"); //exits file 
				memset(file, '\0', sizeof(file));//clearing file buffer 		 
			}//end of file validity check 
			else 
			{//else for valid file length 
				printf("Incoming File Length: %d\n", incoming_file_length); //exits file 
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
					
				memset(file, '\0', sizeof(file));//clearing file buffer 					
				
				FILE *fp; 
				fp = fopen(new_file, "w+"); 
				int iterations = 0;
				int iteration_no = 0; 
				printf("Progress: Receiving File Commenced\n");
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
									received_packet[j] = received_packet[j] ^ (0xFFFF); //decrypting
								}
								printf("Size of Packet Received: %ld\n", sizeof(received_packet));
								//writes to the file 
								if(fwrite(received_packet, 1, MAX_FILE_BUFFER_SIZE, fp)<=0)
								{
									printf("Error: Writing to Receipient File\n");
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
									last_packet[j] = last_packet[j] ^ (0xFFFF); //decrypt file
								}

								printf("Size of Packet Received: %ld\n", sizeof(last_packet));
								if(fwrite(last_packet, 1, last_packet_size, fp)<=0)
								{
									printf("Error: Writing to recipient file\n");
								} 
								
								printf("Size of Packet Received: %ld\n", sizeof(received_packet));
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
				printf("Progress: Finished Receiving\nPUT FILE COMMAND COMPLETED\n");	
				fclose(fp);	
				reliability_bit = 1; 
			}//end of received valid file 
		}//end of put command
				


/*--------------------LS---------------------------------*/			
		if (strcmp("ls", command) == 0)
		{//start ls
			struct sockaddr* remote = (struct sockaddr*)&client_address; 
			returnvalue = lsCommand(socketfiledescriptor, remote); 
			if(returnvalue == FAILURE)
			{
				printf("Error: Action Failed\n"); 
			}
			else
			{
				printf("\nLIST ROOT DIRECTORY COMPLETED\n"); 
			}
			memset(command, '\0', sizeof(command));
		}//end ls
		


/*---------------------------DEL--------------------------*/
		if (strcmp("delete", command) == 0)
		{//start delete command
			
			struct sockaddr* remote = (struct sockaddr*)&client_address; 
			returnvalue = deleteCommand(socketfiledescriptor, remote, file);  
			int r; 
			if(returnvalue == FAILURE)
			{
				printf("Error: Action Failed\n"); 
			}
			else 
			{
				printf("\nDELETE FILE COMMAND COMPLETED\n");
			}
			memset(file, '\0', sizeof(file));
			memset(command, '\0', sizeof(command));
		}//end delete command


/*---------------------------EXIT--------------------------*/
		if (strcmp("exit", command) == 0)
		{//start exit
			returnvalue = exitCommand(socketfiledescriptor); 
			int r; 
			if(returnvalue == FAILURE)
			{
				printf("Error: Action Failed\n"); 
			}
			else
			{
				printf("\nEXIT SERVER SOCKET COMPLETED\n\n"); 
				memset(command, '\0', sizeof(command));
				return SUCCESS; 
			}
		}//end exit 
		
		
		memset(command, '\0', sizeof(command));//clearing command buffer 
		memset(file, '\0', sizeof(file));//clearing file buffer 
	}//end while 
	
return 1; 
}//end main
	 
		


