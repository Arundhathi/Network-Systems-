/* 
 * Test Code: Http based web server 
 * Subject: Network Systems 
 * Name: Arundhathi Swami 
 * Date: 13th October 2017
 *ref: https://blog.abhijeetr.com/2010/04/very-simple-http-server-written-in-c.html 
 */


#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h> 
#include<sys/wait.h>
#include<arpa/inet.h>
#include<netdb.h> 
#include<signal.h> 
#include<fcntl.h> 
#include<stdint.h>
#include"doublelinkedlist.h"
#include<stdlib.h>
#include<pthread.h>


/*Macro Definitions*/
#define MAX_CLIENT_CONNECTIONS 1000
#define BYTES 1024 
#define BUFFER_SIZE 512

/*Error Handling Macros*/ 

//400
char error_400[] = 
"HTTP/1.1 400 Bad Request Error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 400: Bad Request Error. REASON: Request Not Supported</h1><br>\r\n";

char error_400_packet[250]; //final packet to send error 400

//404
char error_404[] = 
"HTTP/%0.1f 404 Not Found Error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 404: Not Found. REASON: URL does not exist</h1><br>\r\n"; 

char error_404_packet[250]; //final packet to send error 404
  

//500 
char error_500[] = 
"HTTP/%0.1f 500 Internel Server Error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 500: Internal Server Error. REASON: File Format Not Supported</h1><br>\r\n";

char error_500_packet[250];//final packet to send error 500


//501
char error_501[] = 
"HTTP/%0.1f 501 Not Implemented\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 501: Not Implemented. REASON: Only GET and POST Methods Implemented</h1><br>\r\n";

char error_501_packet[250];//final packet to send error 501

/*User defined data type to store valid web pages*/
typedef struct webpagelist{
  char page1[20]; 
  char page2[20];
  char page3[20];
} wlist; 


/*Global Variables*/
char ROOTDIR[100];
int listenfd, clients[MAX_CLIENT_CONNECTIONS];
FILE *fptr;
char dot[] = ".";
char* ext;
node* headRef = NULL;
char* rootdir;


/*Function Definitions*/

/*Function: Count Number of Lines in configuration file
 * Input: File pointer to conf file 
 * Output: int to number of lines in file 
 */
int countLinesInFile(FILE *fp)
{
  fp = fopen("ws.conf", "r"); 
  if (fp == NULL)
  {
    printf("Stupid File\n"); 
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

/*Function:Extract port number from the conf file  
 * Input: char pointer to particular line from conf file containing port number 
 * Output: int to port number
 */
int parsePortNumber(char* line)
{
  printf("Progress: Getting Port Number\n"); 
  int port; 
  char* portnumber; 
  portnumber = strtok(line, " "); 
  portnumber = strtok(NULL, ""); 
  port = atoi(portnumber); 
  return port; 
}

/*Function:Extract root directory from the conf file  
 * Input: char pointer to particular line from conf file containing root directory 
 * Output: char pointer to root directory
 */
char* parseDocumentRoot(char* line)
{
  printf("Progress: Getting Document Root\n"); 
  char* rootpwd; 
  rootpwd = strtok(line, " ");
  rootpwd = strtok(NULL, "\"");
  return rootpwd; 
}

/*Function:Extract valid web pages from the conf file  
 * Input: char pointer to particular line from conf file containing list of web pages 
 * Output: wlist data type containing char arrays as members storing web pages 
 */
wlist parseDefaultWebPage(char* line)
{
  printf("Progress: Getting Default Web Pages\n"); 
  char* webpgs;
  wlist list; 
  webpgs = strtok(line, " "); 
  webpgs = strtok(NULL, " ");
  strcpy(list.page1, webpgs);
  webpgs = strtok(NULL, " ");
  strcpy(list.page2, webpgs); 
  webpgs = strtok(NULL, " ");
  strcpy(list.page3, webpgs);
  return list; 
}

/*Function:Extract port number from the conf file  
 * Input: char pointer to particular line from conf file containing timeout value  
 * Output: int totimeout value
 */
int parseTimeout(char* line)
{
  char* timeout; 
  timeout = strtok(line, " "); 
  timeout = strtok(NULL, " "); 
  timeout = strtok(NULL, ""); 
  int time = atoi(timeout); 
  return time;
}

/*Function: finds out extension of file requested   
 * Input: char pointer to file name  
 * Output: int to status of function
 * */
int return_extension(char* filename)
{
  int i = 0; 
  int length = strlen(filename);
  char extension[20];  
  ext = strtok(filename, "."); 
  ext = strtok(NULL, ".");
  strcpy(extension, ext);
  strcat(dot, extension);
  return 0;
}
  
/*Function: To start the initial server connection in TCP that accepts all other client requests
 * Input Arguments: char * port number //server starts accepting clients at this port number 
 * Retun Type: Void 
 * */
void startServer(char* portno)
{
  struct addrinfo hints, *res, *p;
  printf("Port No Opened: %s\n", portno);
  /*struct addrinfo is a predefined structure in netdb.h
   * members include 
   * ai_flags - input flags 
   * ai_family - address family of socket
   * ai_socktype - socket type 
   * ai_protocol - protocol of socket 
   * socklen_t addrlen - length of socket address
   * struct sockaddr *ai_addr - socket address of socket
   * struct addrinfo *ai_next - pointer to next client in list
   * 
   * hints parameter specifies the preferred socket type or protocol. A NUll hints specifies that client sockets of any protocol are acceptable
   */
  
  //get addrinfo for the host 
  memset(&hints, 0, sizeof(hints)); //clears tthe hints struct of residual values 
  hints.ai_family = AF_INET; //internet protocol family 
  hints.ai_socktype = SOCK_STREAM; //TCP 
  hints.ai_flags = AI_PASSIVE; 


  /*int getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res); 
Function: Given node and service identify an internet host and a servie. This function returns one or more addrinfo structures which contain an internet address that can be specified in a call to bind() or connect()
Function is reentrant and allows programs to eliminate IPv4 vs IPv6 dependencies. 

AI_PASSIVE flag : node parameter is null, the returned socket address will be suitable for bind() ing a socket that will accept() connections, the returned socket address will contain the wildcard address (INADDR_ANY for IPv4 and IN6ADDR_ANY_INIT for IPv6). Wildcard address is used by applications that intend to accept connections on any of the hosts network addresses. if node isnt null, AI_PASSIVE is ignored. 

If AI_PASSIVE isnt set in hints, returned socket address will be suitable for use with connect(), sendto(), or sendmsg(). If node is NULL then the address woll be set to loopback interface address. (INADDR_LOOPBACK). Used for applications that intend to communicate with peers on the same host
*/
  if(getaddrinfo(NULL, portno, &hints, &res) != 0)
  {
    perror("Error: Unable to create socket\n");
    exit(1); 
  }

  for (p= res; p!=NULL; p=p->ai_next) //for all the clients 
  {
    listenfd = socket(p->ai_family, p->ai_socktype, 0);//creates an endpoint 
    /* int socket(int domain, int type, int protocol) : where domain is the internet family (AF_INET), type is Socket type for tcp/udp etc,protocol can be sock_stream or sock_dgram. ON success a valid file descriptor is returned. Errors return -1*/ 
    if (listenfd == -1) continue; //wait until valid socket file descriptor is obtained 
    struct timeval timeout; 
  	 timeout.tv_sec = 10; 
  	 timeout.tv_usec = 0; 
  	 if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) <0)
    {
      printf("Error: Timeout Not Set\n"); 
    }
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break; 
    /* int bind(int sockfd, const struct sockaddr* address, socklen_t addresslength); 
     * Function is to assigns the address to the socket names using the file descriptor. "Assigning a name to a socket"*/ 

  }

  if(p==NULL)
  {
    perror("Error: socket() or bind()\n");
    exit(1); 
  }
  
 freeaddrinfo(res); //deallocates addrinfo struct 

 //listen for incoming connections 
 if(listen(listenfd, 1000000) != 0)
 {
   perror("Error: listen() error\n"); 
   exit(1); 
 }
}


/* Function: Client response function to get and post methods  
 * Input Parameters: int client number and char pointer to root directory
 * Return Type: void 
 * */
void respond(int clientno, char* root)
{
  char msg[99999], post_msg_rcvd[99999], *reqline[3], datatosend[BYTES], path[99999];
  int rcvdmsg, bytes_to_read;
  FILE *fp; 

  memset((void*)msg, (int)'\0', 99999);

  while( recv(clients[clientno], msg, 99999, 0)>=0) //while number of bytes received is positive
  {
   strncpy(post_msg_rcvd, msg, strlen(msg)); 
  /* size_t recv(int sockfd, void *buf, size_t bufferlength, int flags)
   * Function: Used to receive msgs from a socket
   * Return: Length of message successfully received or -1 if error occurs returns 0 if the socket is shut down gracefully.
   * Characteristics: Blocking, Normally used only on a connected socket
   * Flags argument is to set characteristics such as MSG_DONTWAIT(Enables non blocking call), MSG_ERRQUEUE (specifies that queued errors should be received from the socket error queue. 
   */

    printf("Progress: Message Received\n"); 
    printf("Message: %s\n", msg); 
    reqline[0] = strtok (msg," \t\n");//tokenizes the entire message to get the method 
    printf("Request Received is: %s\nLength of Msg: %d\n", reqline[0], strlen(reqline[0]));  
    if((strncmp(reqline[0], "GET\0", 4) != 0 ) && (strncmp(reqline[0], "POST\0", 5) != 0)) //checks if first element of the recvd msg is a get request pr post reuest
    {//if not get or post 
      printf("Error: Inside not get or put\n");
      if ( (strncmp(reqline[0], "PUT\0", 4) == 0) || (strncmp(reqline[0], "DELETE\0", 7) == 0) || (strncmp(reqline[0], "TRACE\0", 4) == 0) || (strncmp(reqline[0], "PATCH\0", 4) == 0) || (strncmp(reqline[0], "OPTIONS\0", 4) == 0) || (strncmp(reqline[0], "CONNECT\0", 4) == 0) )
      {
		  printf("Error: Unsupported Request\n");
        sprintf(error_501_packet, error_501, 1.1); //sends error 501 in http 1.1 version
        int bytessent = send(clients[clientno], error_501_packet, strlen(error_501_packet), 0);
      }
		else
		{
        printf("Error: Invalid Request\n");
        sprintf(error_400_packet, error_400, "Invalid Request"); //sends error 400 in http 1.1 version 
        int bytessent = send(clients[clientno], error_400_packet, strlen(error_400_packet), 0);
      }
      shutdown(clients[clientno], SHUT_RDWR); //shuts down both read and write functionalities for the client 
  		close(clients[clientno]);
  		clients[clientno] =-1; //resets client count 
  		return;
    }//end of not get or post
    reqline[1] = strtok(NULL, " \t");//gets second and third token from request
    reqline[2] = strtok(NULL, " \t\n"); 
    printf("Reqline 1: %s\n Reqline 2: %s\n", reqline[1], reqline[2]); 
    if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp(reqline[2], "HTTP/1.1", 8)  != 0)
    {//checks if http request if of type 1.0 or 1.1
      printf("Error: Invalid HTTP Version\n");
      sprintf(error_400_packet, error_400, "Invalid HTTP Version"); 
      int bytessent = send(clients[clientno], error_400_packet, strlen(error_400_packet), 0);
      shutdown(clients[clientno], SHUT_RDWR); //shuts down both read and write functionalities for the client 
  		close(clients[clientno]);
  		clients[clientno] =-1; //resets client count 
  		return; 
    }
    else
    {//valid http version
      if ((strncmp(reqline[1], "/\0", 2) == 0) || (strncmp(reqline[1], "/index.html\0", 11) == 0) || (strncmp(reqline[1], "/index.htm\0", 10) == 0) || (strncmp(reqline[1], "/index.ws\0", 9) == 0))//checks for valid start pages
      {
        reqline[1] = "/index.html"; //if file component is null, it will open index,html by default 
      }
      char fileformat[10]; 
      strcpy(fileformat, reqline[1]); 
      printf("Progress: File Requested = %s\n", fileformat); //sets file requested
      int extension_validity_check; 
      extension_validity_check = 0; 
		  printf("Root Directory is: %s\n", root); 
      strcpy(path, root); 
      strcpy(&path[strlen(root)], reqline[1]); 
      printf("Complete Path for File Required: %s\n", path); 
      return_extension(fileformat);
  		printf("Extension: %s\n", dot); //dot contains file extension with the . appended 
  		int check = search_node(headRef, dot); 
 		  printf("Extension Check: %d\n", check); 
      if (check == 0)
      {//valid file extension
        printf("Error: Unsupported File Format\n");
        if (strncmp(reqline[2], "HTTP/1.1", 8) == 0)
        {
          sprintf(error_500_packet, error_500_packet, 1.1);
          printf("Error: Unsupported File Format\n"); 
          int bytessent = send(clients[clientno], error_500_packet, strlen(error_500_packet), 0);
        }
        if (strncmp(reqline[2], "HTTP/1.0", 8) == 0)
        {
          sprintf(error_500_packet, error_500_packet, 1.0);
          printf("Error: Unsupported File Format\n"); 
          int bytessent = send(clients[clientno], error_500_packet, strlen(error_500_packet), 0);
        }
        shutdown(clients[clientno], SHUT_RDWR); //shuts down both read and write functionalities for the client 
  		  close(clients[clientno]);
  		  clients[clientno] =-1; //resets client count 
  		  return; 
      }//valid file extension

      /*create header variables*/
      char header[100]; 
      char* headerptr = NULL; 
      
      if ((fp = fopen(path, "r"))== NULL) //checks if file is available
      {
         printf("Error: File Not Found in Root Directory\n"); 
         if (strncmp(reqline[2], "HTTP/1.1", 8) == 0)
         {
          sprintf(error_404_packet, error_404_packet, 1.1);
          printf("Error: File Not Found in Root Directory\n"); 
          int bytessent = send(clients[clientno], error_404_packet, strlen(error_404_packet), 0);
         }
         if (strncmp(reqline[2], "HTTP/1.0", 8) == 0)
         {
          sprintf(error_404_packet, error_404_packet, 1.0);
          printf("Error: File Not Found in Root Directory\n"); 
          int bytessent = send(clients[clientno], error_404_packet, strlen(error_404_packet), 0);
         }
         shutdown(clients[clientno], SHUT_RDWR); //shuts down both read and write functionalities for the client 
  		   close(clients[clientno]);
  		   clients[clientno] =-1; //resets client count 
  		   return; 
      } 
      else
      {
         printf("Progress: File Found\n"); 
         int flen;
         fseek(fp, 0, SEEK_END);
         flen = ftell(fp); 
         fseek(fp, 0, SEEK_SET); 
         size_t bytes; 
         //check if method is get or post to set the right header 
         if (strncmp(reqline[0], "GET\0", 4) == 0)
         {//get
            if (strncmp(reqline[2], "HTTP/1.1", 8) == 0)
            {
              sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d Bytes\r\n\r\n",dot,flen); 
            }
            else if (strncmp(reqline[2], "HTTP/1.0", 8) == 0)
            {
              sprintf(header, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %d Bytes\r\n\r\n",dot,flen);
            }
         }//get
         if (strncmp(reqline[0], "POST\0", 5) == 0)
         {//post
            //get string to be posted to page 
            char* post_msg;
            
            post_msg = strstr(post_msg_rcvd, "\r\n\r\n"); 
            
            post_msg = post_msg + 4; //move the pointer but not the value it points to
            
            if (strncmp(reqline[2], "HTTP/1.1", 8) == 0)
            {
              sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d Bytes\r\n\r\n<h1>%s</h1>\r\n",dot,flen, post_msg); 
            }
            else if (strncmp(reqline[2], "HTTP/1.0", 8) == 0)
            {
              sprintf(header, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %d Bytes\r\n\r\n<h1>%s</h1>\r\n",dot,flen, post_msg);
            }
         }//post
         printf("Progress: Header Constructed: %s\n", header);
         //send header 
			   bytes = send(clients[clientno], header, strlen(header), 0);
         //send file
         while((bytes_to_read = fread(datatosend, 1, BYTES, fp))>0)
         {
           bytes = write(clients[clientno], datatosend, bytes_to_read);
          
         }
         fclose(fp);
       }
       
     }//valid http version ends 
   }//else  ends 
   shutdown(clients[clientno], SHUT_RDWR); //shuts down both read and write functionalities for the client 
   close(clients[clientno]);
  	clients[clientno] =-1; //resets client count
}//function ends 


/*Main Function*/
int main(int argc, char* argv[])
{//start of main 
  struct sockaddr_in clientaddr; //declaring two variables of type sockaddr_in 
  
  socklen_t addrlen; 
  char c; 
  int portno; 
 
  
  //Default values PATH = ~/ and PORT = 10000
  char PORT[10];
   
  //ROOT = getenv("PWD"); //syscall to execute PWD and get current directory absolute path 
  //strcpy(PORT, "10000"); //assigns PORT with value 10000

  int slot = 0; //initial client connection number
  
    
  FILE *fp = fopen("ws.conf", "r");
  if(fp==NULL)
  {
    printf("Error: File Doesnt Exist\n");
  }
  else
  {
    printf("Progress: Config File Parsing Started\n");

    int lines = 0;
    lines = countLinesInFile(fp); 
    printf("Lines in File: %d\n", lines);
    char* word;
    int now = 0; 
    char parsesegment[315];
      
    while(fgets(parsesegment, sizeof(parsesegment), fp))
    {  
      word = strtok(parsesegment, "\n");
      printf("%s\n", word);
      if (strncmp(word, "#service port number", 20) == 0)
      {
        char* value;
        char partsegment[25]; 
        fgets(partsegment, sizeof(partsegment), fp);
        value = strtok(partsegment, "\n");        
		  portno = parsePortNumber(value); 
        sprintf(PORT, "%d", portno); //stores int type as char type so it can be used in startserver function
        printf("Port No: %d\n", portno); 
      }
      else if (strncmp(word, "#document root", 14) == 0)
      {
        char* value;
        char partsegment[70]; 
        fgets(partsegment, sizeof(partsegment), fp);
        value = strtok(partsegment, "\n");
        rootdir = parseDocumentRoot(value);
        strcpy(ROOTDIR, rootdir);
        printf("Document Root: %s\n", rootdir); 
      }
      else if (strncmp(word, "#default web page", 17) == 0)
      {
        char* value;
        char partsegment[50]; 
        fgets(partsegment, sizeof(partsegment), fp);
        value = strtok(partsegment, "\n");
        wlist pages = parseDefaultWebPage(value);
        printf("Default webpage %d: %s\n", 1, pages.page1);
        printf("Default webpage %d: %s\n", 2, pages.page2); 
        printf("Default webpage %d: %s\n", 3, pages.page3);
      }
    
      else if (strncmp(word, "#Content-Type which the server handles", 38) == 0)
      {
        char* value;
        char partsegment[50]; 
        int iteration = 0;
        
        char* extension; 
        char* description;
        while(iteration<9)
        {
          fgets(partsegment, sizeof(partsegment), fp);
          value = strtok(partsegment, "\n");
          extension = strtok(value, " "); 
          description = strtok(NULL, ""); 
          int ret = add_node(&headRef, description, extension, 0);  
          //printf("%d\n", headRef);
          memset(partsegment, '\0', sizeof(partsegment));
          iteration++;
        }
        print_list(&headRef);
        
      }
      else if (strncmp(word, "#connection timeout", 19) == 0)
      {
        char* value;
        char partsegment[50]; 
        fgets(partsegment, sizeof(partsegment), fp);
        value = strtok(partsegment, "\n");
        int timeout = parseTimeout(value); 
        printf("Timeout For Inactive Client: %d\n", timeout);
      }
      memset(word, '\0', sizeof(word));
       
	}//while ends
 }//else ends 
 
  //at this point if the right command line args have been passed, the server has been started at port 10000 or any of the ports mentioned above 
  printf("\nProgress: Server started at Port No: %d with Root Directory: %s\n", portno, ROOTDIR); 
  //printf("%s\n", root);
  //to initialise the server, we have to establish 0 clients by setting the client list arra to -1s
   
  for(int i=0; i<MAX_CLIENT_CONNECTIONS; i++)
  {//for loop 
    clients[i] = -1; 
  }//end for 

  
  //starting server with user configured features with the original socket that is responsible for accepting all other queued connections
  startServer(PORT);

   pid_t id;
  //accepts connection requests to he server 

  while (1) //server needs to be accepting connections at all times 
  {//server main while loop 
    int returnvalue = 0; 
    addrlen = sizeof(clientaddr); 
    clients[slot] = accept(listenfd, (struct sockaddr*) &clientaddr, &addrlen);
/*accept( int socket, struct sockaddr *restrict address, socklen_t *restrict address, socklen_t *restrict addresslength)
 * Function: extracts the first connection on the queue of pending connections, creates a  new socket with the same socket type protocol and address family as the specified socket, and allocates a new file descriptor for that socket. 
 *
 * Arguments: 
 * 1.  socket - specifies a socket was created with socket() syscall, bound to an address with bind() syscall, has issued a successful syscall to listen() syscall 
 * 2. address - pointer to a sockaddr structure where the address of the connecting socket shall be returned 
 * 3. addresslength - points to a socklen_t type structure which on input specifies the length of the supplied sockadddr structure and  on output specifies the length of the stored address
 *
 * if the queue is empty, accept() blocks until the next connection arrives. 
 *
 * Return: Upon successful connection, accept() returns non-negative file descriptor of the accepted socket. Error -1 will be returned. 
 */
    if (returnvalue < 0)
    {
      printf("Error: Queued socket not accepted by server.\n"); 
    }
    else 
    {//else loop
      int forks; 
      id = fork();
      if (id == 0) //creates new thread for every accepted client , 0 means successfully created
      {
        printf("Forks Created: %d\n", forks); 
        respond(slot, ROOTDIR); 
        forks++;
        exit(0); 
      }
    }//else loop 
    waitpid(id, NULL, 0);
    
    while(clients[slot] !=  -1)
    {//while 
      slot = (slot+1)%MAX_CLIENT_CONNECTIONS; //updates slot number while checking for maximum clients allowed
    }//while
  }//main while 

  return 0; 
}//end main 


