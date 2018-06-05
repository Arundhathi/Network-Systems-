/*#/****************************
# * File: Proxy Webserver
# * Subject: Network Systems 
# * Author: Arundhathi Swami 
# * Date: December 13 2017 
# * *************************/ 



#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<stdbool.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<fcntl.h>
#include<sys/time.h>
#include<openssl/md5.h>
#include<time.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<dirent.h>

#define CACHE_DIR "cachedir"
#define MAX_CONNECTIONS 10000
#define MAX_BUFFER_SIZE 2048
#define MAX_FILE_SIZE 1000000
#define MAX_FILENAME_SIZE 1000
#define PATH_SIZE 1000
#define GEN_BUFF_SIZE 1000
#define PRESENTINCACHE 45
#define ABSENTINCACHE 50
#define BLACKLISTED 55
#define NOTBLACKLISTED 60
#define FINISHEDFILETRANSFER 65
#define INCOMEPLETEFILETRANSFER 70
#define FAILED 0
#define SUCCESS 1

/*User Defined Data types*/ 
typedef enum BlacklistStatus{
  ACCEPT, 
  REJECT
} blacklist_t;

typedef struct WebpageData {
  uint32_t port_num;
  char ip_addr[20];
  char client_command[10];
  char domain_name[100];
  char filename[1000];
  char http_version[20];
  char domain_name_hash[(MD5_DIGEST_LENGTH*2)+1];
  bool cache_file;
} webpagedata_t;

typedef struct __cache_file_operations {
  char pwd_string[100];
} cache_info;

/*array storing client connections*/
int32_t client_connections[MAX_CONNECTIONS];
/* timeout value*/
uint16_t timeout = 0;
/*blacklist variables*/
typedef struct blocked_file
{
    char blocked_list[10][50];
}blocked;

blocked blacklist;

uint8_t blacklistno = 0;
/*ip caching array*/
struct hostent ipcache[10];




/*error handling arrays*/
char unsupportedmethod[GEN_BUFF_SIZE] = "<html><body><H1>ERROR 400: (BAD REQUEST)\nOnly GET Method Supported </H1></body><html>"; 

char unsupportedhttpversion[GEN_BUFF_SIZE] = "<html><body><H1>ERROR 400: (BAD REQUEST)\nOnly HTTP 1.0 and HTTP 1.1 versions Supported </H1></body><html>"; 

char wrongwebpage[GEN_BUFF_SIZE] = "<html><body><H1>ERROR 400: (BAD REQUEST)\nOnly http:// format Supported </H1></body><html>"; 

char accesserror[GEN_BUFF_SIZE] = "<html><body><H1>ERROR: 403 (FORBIDDEN)\nAccess Denied as Black Listed </H1></body></html>";

char pagenotfound[GEN_BUFF_SIZE] = "<html><body><H1>ERROR: 404 (NOT FOUND)\n Reason URL does not exist or is inaccessible</H1></body></html>";


uint8_t ipcachingandblockcheck(char * reqwebpage, char* ipaddress)
{
  // printf("Host name in hostnametoip function: %s\n", hostname);
  struct hostent *hostentity;
  struct in_addr **webpageaddresslist;
  static uint16_t currentcachedipcount = 0;
 	
#ifdef ipcache
   /* Iterate through cache and check if ip is alread in there*/
  for(int i = 0; i<currentcachedipcount; i++) 
  {

   
    if(!strcmp(reqwebpage, ipcache[i].h_name)) /* if not present in cache*/
    {
      webpageaddresslist = (struct in_addr **) ipcache[i].h_addr_list; /*add ip to cache */
      for(int i = 0; webpageaddresslist[i] != NULL; i++) 
      {
        strcpy(ipaddress, inet_ntoa(*webpageaddresslist[i]) );/*copy ip in given array*/
      }
     
     }
	}
   if ((hostentity = gethostbyname(reqwebpage)) == NULL) 
   {
    // get the host info
    herror("gethostbyname");
    //return FAILED;
   }

  /* cache the struct */
  memcpy(&ipcache[currentcachedipcount++], hostentity, sizeof(struct hostent));
#endif 
    
	for(int j =0 ;j<blacklistno ; j++)
      {
        if(strncmp(reqwebpage,blacklist.blocked_list[j],strlen(reqwebpage)) == 0)
        {
          printf("\nERROR: Accessing BlackListed File\n");
         // write(newsockfd,blocked_reqeust,strlen(blocked_reqeust));
          return BLACKLISTED;
        }
      }
	return NOTBLACKLISTED;
}


/*Function to set up connection between the server and proxy server*/
uint8_t EstablishServer(int *serverendsocket, struct sockaddr_in *serveraddress, int serveraddrlength, int proxyserverport)
{
  if(serveraddress == NULL)
  		return FAILED;
  (*serverendsocket) = socket(AF_INET, SOCK_STREAM, 0);
  
  /*allow reuse of port number*/
  setsockopt((*serverendsocket), SOL_SOCKET, SO_REUSEADDR, &(int){1} , sizeof(int));
  
  /* set the parameters for the server */
  serveraddress->sin_family = AF_INET;
  serveraddress->sin_port = htons(proxyserverport);
  serveraddress->sin_addr.s_addr = INADDR_ANY;

  /* Bind the server to the client */
  int ret = bind(*serverendsocket, (struct sockaddr *)serveraddress, serveraddrlength);
  if(ret < 0) {
    perror("ERROR:bind()\n");
    close(*serverendsocket); 
    exit(1);
  }

  /* Start listening for the connection on the socket */
  ret = listen(*serverendsocket, 5);
  if(ret < 0) {
    perror("ERROR:listen()\n");
    exit(1);
  }
  return SUCCESS;
}

/* Function to compute the md5 hash of a domain name page*/
uint8_t computemd5hash(char *hashstore, char* instring)
{
  unsigned char digest[MD5_DIGEST_LENGTH]; 
  memset(digest, '\0', sizeof(digest));
  /* creates an instance of an MD5 context structure*/
  MD5_CTX md5_ctnxt;
  MD5_Init(&md5_ctnxt);
  MD5_Update(&md5_ctnxt, instring, strlen(instring));
  MD5_Final(digest, &md5_ctnxt);
  
   /* Stores the MD5 hash as a string */
  for(int i = 0; i<MD5_DIGEST_LENGTH; i++) {
    sprintf(&hashstore[i*2], "%02x", digest[i]); 
  }
  return SUCCESS;
}

/*Checks if there is a cache directory, if not makes one*/
int cachedirectoryexistence(const char *dir)
{
  struct stat st = {0};
  
  if(stat(dir, &st) == -1) {
    printf("%s directory does not exist, create now!\n", dir);
    int ret = mkdir(dir, 0777);
    if(ret < 0) {
      perror("MKDIR");
      return SUCCESS;
    }
  }

  return SUCCESS;
}

/*Function to get md5sum of opening webpage*/
char *getmd5sum(const char *str, int length) 
{
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
}

/*function to perform link prefetching of the https links i.e predict link withot entering the whole link*/ 
/* process: checks for the content in href:"http://"
 * if it exists, copies the url and prepares to extract it from the server
 * Return: void 
 * Formal Arguments: char* path: path to the particular file, int socketfiledescriptor: file descriptor for the client
 */
void linkprefetch(char* path, int socketfiledescriptor)
{
  /*buffer initializations*/ 
  char sendbuff[MAX_FILE_SIZE]; 
  memset(sendbuff, '\0', sizeof(sendbuff)); 
  
  char recbuff[GEN_BUFF_SIZE]; 
  memset(recbuff, '\0', sizeof(recbuff)); 
  
  char pathtocfile[PATH_SIZE]; 
  memset(pathtocfile, '\0', PATH_SIZE); 
  
  char prefetchsection[PATH_SIZE]; 
  memset(prefetchsection, '\0', PATH_SIZE); 

  char newpath[PATH_SIZE]; 
  memset(newpath, '\0', sizeof(newpath)); 

  char newlink[PATH_SIZE]; 
  memset(newlink, '\0', sizeof(newlink)); 

  char checkpresence[GEN_BUFF_SIZE]; 
  memset(checkpresence, '\0', sizeof(checkpresence)); 

  /*variables needed to check prefetch matching*/ 
  int index, openfile, readbytes, flag, num; 
  char* md5sumoffile; 
  FILE* fp = NULL;
  char *hrefcheck; 
  char *newhrefcheck; 
  char *newlinkcheck;
  
  struct sockaddr_in hostaddr; 
  struct hostent* hostentity;
  int p = 9;

  /* open file from current path*/ 
  openfile = open(path, O_RDONLY); 
  if(openfile == -1)
    printf("ERROR: opening file\n"); 
  /*read file*/ 
  readbytes = read(openfile, sendbuff, sizeof(sendbuff)); 
  if(readbytes<0)
    printf("ERROR: Cant read file\n"); 

  if ((hrefcheck =  strstr(sendbuff, "href=\"http://")) != NULL)
  {
    while((hrefcheck= strstr(hrefcheck, "href=\"http://")) )
    {
     hrefcheck += 13; 
     index = 0;
     while(*hrefcheck != '"')
     {
       newlink[index] = *hrefcheck; 
       printf("%c", *hrefcheck); 
       hrefcheck++;
       index++; 
     }
     newhrefcheck = hrefcheck; 
     newlink[index] = '\0'; 

     //computing md5sum of file 
     strcpy(checkpresence, newlink); 
     md5sumoffile = getmd5sum(checkpresence, strlen(checkpresence)); 
     printf("md5 sum of prefetch link: %s\n", md5sumoffile); 

     /*navigate to cache dir folder and search for prefectched link*/     
     strcpy(pathtocfile, "./cachedir"); 
     strcat(pathtocfile, md5sumoffile); 
     strcat(pathtocfile, ".html"); 
     printf("path to cached file: %s\n", pathtocfile); 
     newlinkcheck = strstr(newlink, "/"); 
     if(newlinkcheck == NULL)
       continue;
     if(newlinkcheck != NULL)
     {
       printf("Progress: Link being prefecthed\n"); 
       strcpy(newpath, newlinkcheck); 
       printf("Prefecthed path: %s\n", newpath); 
     }
     *newlinkcheck = '\0'; 
     hrefcheck = newhrefcheck+1; 
     /*send the file to the client from the server*/
    }
  }
}


/*Function to connect to http webserver and relay file to client*/
uint8_t sendfromwebserver(int client_sock_num, char domainwebpage[], char* sendrequesttoserver, uint8_t* cachereqd, char hashedfilename[], char filename[], char httpversion[])
{
  /* Construct the GET request to send to the webserver */
   printf("Sending Request to Remote Server: %s\n", sendrequesttoserver);
  printf("Client fd %d\n", client_sock_num);
  
  
  /* Open a connection to the webserver */
  struct hostent *host_connection;
  struct sockaddr_in host_address;
  
  host_connection = gethostbyname(domainwebpage);
  if(!host_connection) 
  {
    perror("HostConnection");
    return FAILED;
  }

  host_address.sin_family = AF_INET;
  host_address.sin_port = htons(80);
  memcpy(&host_address.sin_addr, host_connection->h_addr, host_connection->h_length);
  
  socklen_t addr_len = sizeof(host_address);
  //printf("IP IN sendinf from webserver function: %s\n",	inet_ntoa(host_address.sin_addr));
  int server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(server_sock < 0) {
    perror("SOCKET");
    return FAILED;
  }

  int optval = 1;

  /* Set socket properties */
  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, 4);

  int connect_ret = connect(server_sock, (struct sockaddr *)&host_address,addr_len);
  if(connect_ret < 0) {
    perror("CONNECT: ");
    return FAILED;
  }

   /* Construct the GET request to send to the webserver */
  char server_request[300];
  memset(server_request, '\0', sizeof(server_request));

  if(filename[0] == '\0') 
  {
    sprintf(server_request, "GET / %s\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n", httpversion, domainwebpage);
  } 
  else 
  {
    sprintf(server_request, "GET /%s %s\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n",filename, httpversion, domainwebpage);
  }
  
  printf("server_request: %s\n", server_request);
	
  int send_bytes = send(server_sock, server_request, sizeof(server_request), MSG_NOSIGNAL);
  if(send_bytes < 0) {
    perror("SEND:");
    return FAILED;
  }
  else 
  {
		printf("Progress: Sent GET Method to Server\n");
  }
  /* Get the response from the webserver and print it on the console */
  int32_t recv_bytes = 0;
  
  char server_recv_buffer[MAX_BUFFER_SIZE];
  memset(server_recv_buffer, '\0', sizeof(server_recv_buffer));

  /* Check to see if the file needs to be cached */
  FILE *cachefile = NULL;
  char pathtohashfile[1000];
  if(*cachereqd == ABSENTINCACHE) 
  {
    
    memset(pathtohashfile, '\0', sizeof(pathtohashfile));
    
    sprintf(pathtohashfile, "./%s/%s", CACHE_DIR, hashedfilename);  
    printf("Progress: Writing new hash file to cache at :%s\n", pathtohashfile); 
    cachefile = fopen(pathtohashfile, "wb");
    if(cachefile == NULL) 
    {
      perror("FILE_ERROR: ");
      return FAILED;
    }
  }

  do 
  {
    memset(server_recv_buffer, '\0', sizeof(server_recv_buffer));
    recv_bytes = recv(server_sock, server_recv_buffer, sizeof(server_recv_buffer), 0);
    if(recv_bytes <= 0)
	 {
		 perror("RECV:"); 
    }
    /* Send the data over to the client via the client socket */
    send_bytes = send(client_sock_num, server_recv_buffer, recv_bytes, MSG_NOSIGNAL);  
	 if(send_bytes <= 0)
	 {
		 perror("SEND:"); 
    }
    /* Write the buffer to the cache file */
    fwrite(server_recv_buffer, 1, recv_bytes, cachefile);
    
    //printf("%s\n", server_recv_buffer);
  } while(recv_bytes);

  *cachereqd = PRESENTINCACHE;
  printf("\n\nStarting the Prefeting operation... \n\n");
  linkprefetch(pathtohashfile,client_sock_num);
  printf("linkPrefetching Done\n" );

  fclose(cachefile);

  return SUCCESS;
}


/*peruses cache to calculate timeout */
uint8_t cachetimeoutcheck(char* searchforfile, uint8_t* cachereq)
{
  cache_info file_cache;
  memset(&file_cache, '\0', sizeof(file_cache));

  /* Check for the existance of the cache folder and create it if required */
  if(getcwd(file_cache.pwd_string, sizeof(file_cache.pwd_string)) != NULL) {
    //printf("file_cache.pwd_string: %s\n", file_cache.pwd_string);
    cachedirectoryexistence(CACHE_DIR);
  }

  /* Check if the file is present in the folder or not 
   * and if yes, get the time that the file was create 
   */
  char file_path[50];
  memset(file_path, '\0', sizeof(file_path));

  /* Store the file name as its hash value */
  sprintf(file_path, "./cachedir/%s", searchforfile);  
  printf("file_path: %s\n", file_path);

  char c_time[60];
  memset(c_time, '\0', sizeof(c_time));

  struct stat attribute;
  int stat_ret = stat(file_path, &attribute);
  if(stat_ret < 0) 
  {
    printf("ERROR: File does not exist!\n");
    *cachereq = ABSENTINCACHE;
    return ABSENTINCACHE;
  }

  time_t file_creation_time = attribute.st_mtime;
  
  time_t current_time;
  time(&current_time);
  
  time_t time_diff = current_time - file_creation_time;
  
  struct tm *time_info = NULL;
  time_info = localtime(&time_diff);
  
  uint32_t difference = (time_info->tm_min * 60) + (time_info->tm_sec);

  //printf("tm_min: %d\n", time_info->tm_min);
  //printf("tm_sec: %d\n", time_info->tm_sec);

  printf("Set Timeout: %d Time Difference: %d\n", timeout, difference);

  if(difference > timeout) 
  {
    printf("Progress: Timeout occured. Send from WebServer\n");
    *cachereq = ABSENTINCACHE;
    return ABSENTINCACHE;
  } 
  else 
  {
	 printf("Progress: File present in cache. Sending from Cache\n");
    *cachereq = PRESENTINCACHE;
    return PRESENTINCACHE;
  }
}




uint8_t sendfromcache(int client_sock_num, char storefile[], char *temp_buffer)
{
   
  char pathtohashfile[60];
  memset(pathtohashfile, '\0', sizeof(pathtohashfile));

  sprintf(pathtohashfile, "./%s/%s", CACHE_DIR, storefile);  
  printf("<%s>: file_path: %s\n", __func__, pathtohashfile);
  
  /* Open the file for reading */
  FILE *sendfp = NULL;
  sendfp = fopen(pathtohashfile, "rb");
  if(sendfp == NULL)
  {
		printf("ERROR: Couldnt open file\n"); 
 		return FAILED; 
  }  
  int32_t bytes_read = 0;
  int32_t bytes_send = 0;

  char sendbuffer[MAX_BUFFER_SIZE];
  memset(sendbuffer, '\0', sizeof(sendbuffer));
 
  /* Send the cached chunks to the client */
  do 
  {
    bytes_read = fread(sendbuffer, 1, MAX_BUFFER_SIZE, sendfp);
    if(bytes_read <0)
    {
     	perror("FREAD:"); 
    }
    bytes_send = send(client_sock_num, sendbuffer, bytes_read, MSG_NOSIGNAL);
    if(bytes_send <0)
    {
     	perror("SEND:"); 
    }
  } while(bytes_read);

  /* release the file pointer once you're done sending */
  fclose(sendfp);

  return SUCCESS;
}


/*function to decode get request sent in from the client*/ 
uint8_t decoderequest(char request[], char method[], char webpg[], char httptype[], char urlfull[], uint16_t* portno, char clientmethod[])
{
  int flag = 0;
  *portno = 0; 
  sscanf(request, "%s %s %s", method, webpg, httptype);
  strncpy(urlfull, webpg, strlen(webpg)); 
  if(((strncmp(method, "GET", 3) ==0)&&(strncmp(httptype, "HTTP/1.1", 8)==0)) ||(strncmp(httptype, "HTTP/1.0", 8) == 0) && ((strncmp(webpg, "http://", 7) == 0) || (strncmp(webpg, "https://", 8) == 0)))
  {
    //iterate back to get name of the website requested
    for(int i =7; i< strlen(webpg); i++)
    {
      if(webpg[i] == ':')
      {
        flag = 1; 
        break; 
      }
    }
    // printf("In function: %s\n", webpg); 
    char* temp = strtok(webpg, "//"); //string begins from www.
    if(flag == 0)
    {
      *portno = 80; //80 is http port
      temp = strtok(NULL, "/"); 
    }
    else
    {
      temp = strtok(NULL, ":"); 
    }
    sprintf(webpg, "%s", temp); 
    //printf("In function: HOST = %s\n", webpg); 
    if (flag == 1)
    {
      temp =strtok(NULL, "/"); 
      *portno = atoi(temp); 
    }
    char method2[100]; 
    strncpy(method2, method, strlen(method)); 
    strcat(method2, "^]");
    temp = strtok(method2, "//"); 
    temp = strtok(NULL, "/"); 
    if (temp != NULL)
    {
      temp = strtok(NULL, "^]"); 
    }
   //printf("\nIn function: path = %s Port= %d\n", temp, *portno); 
    if(temp!= NULL)
    {
      sprintf(clientmethod, "GET / %s %s\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n", temp, httptype, webpg); 
    }
    else
    {
      sprintf(clientmethod, "GET / %s\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n",httptype, webpg); 
    }
    return 0; 

  }
  else 
  {
    return 1; 
  }
      
 }




void *respondtoclientrequest(void *activeclientfd)
{
  int clientfd = *((int *)activeclientfd);

  char reqbuffer[1000];
  memset(reqbuffer, '\0', sizeof(reqbuffer));
  
  char telnet_string[50];
  memset(telnet_string, '\0', sizeof(telnet_string));
  printf("Client FD%d\n", clientfd);
/*receiving inictial request from client*/
  memset(reqbuffer, '\0', sizeof(reqbuffer));
  int recv_ret = recv(clientfd, reqbuffer, sizeof(reqbuffer), 0);
  if(recv_ret < 0) {
    perror("RECV\n");
    return NULL;
  }
  /*function to decode the request*/ 
  static uint16_t port;
  static char webpage[1000]; 
  static char requesttype[100]; 
  static char httptype[100];
  static char url[1000];
  static char reqsend[1000];
  static char ipaddr[20];  
  static char md5sumofwebpage[100];
  static char extensionfiles[1000]; 
  memset(md5sumofwebpage, '\0', sizeof(md5sumofwebpage)); 
  memset(ipaddr, '\0', sizeof(ipaddr));
  memset(reqsend, '\0', sizeof(reqsend));
  memset(webpage, '\0', sizeof(webpage));
  memset(requesttype, '\0', sizeof(requesttype));
  memset(httptype, '\0', sizeof(httptype));
  memset(url, '\0', sizeof(url));
      
  char temp_buffer[800];
  memset(temp_buffer, '\0', sizeof(temp_buffer));
  printf("\n\n-------------____NEW REQUEST RECEIVED____---------------\n\n");
  printf("Received Request from Client: %s\n", reqbuffer);

  /* Parse the incoming client request */
  sscanf( reqbuffer, "%s %s %s", requesttype,temp_buffer, httptype);
  if(strcmp(requesttype, "GET")) 
  {
    /* The request type is not supported for this implementation */
    int send_ret = send(clientfd, unsupportedmethod, strlen(unsupportedmethod), 0);
    if(send_ret < 0) 
	 {
      perror("SEND");
    }

   close(clientfd);
   pthread_exit(NULL);
    /* Return */
    return NULL;
  }
  if(strcmp(httptype, "HTTP/1.1") &&  strcmp(httptype, "HTTP/1.0")) 
  {
    /* The request type is not supported for this implementation */
    int send_ret = send(clientfd, unsupportedhttpversion, strlen(unsupportedhttpversion), 0);
    if(send_ret < 0) {
      perror("SEND");
    }
    close(clientfd);
    pthread_exit(NULL);

    /* Return */
    return NULL;
  }
  memset(webpage, '\0', sizeof(webpage)); 
  memset(url, '\0', sizeof(url)); 
  /* Cleanup the URL and remove the slashes from it */
  char *temp = strstr(temp_buffer, "//");  
  temp = temp + 2;
  for(int i = 0; i<strlen(temp); i++) {
    if(temp[i] == '/')
      break;
    webpage[i] = temp[i];
  }
  strncpy(url, temp_buffer, strlen(temp_buffer));

 /* Check if there is a filename/path extension to the URL */
#if 1
  int j = 0;
  while(j != strlen(temp)) {
    if(temp[j] == '/') {
      strcpy(extensionfiles, &temp[j+1]);
      /* Change the URL as well */
      //url_request.domain_name[j] = '\0';
      break;
    }
    j++;
  }
#endif
  uint8_t status; 
  printf("Before url_request.ip_addr: %s\n", ipaddr);
  memset(ipaddr, '\0', sizeof(ipaddr));
  /* Get the IP address for the hostname */
  status = ipcachingandblockcheck(webpage, ipaddr);
  if(status == BLACKLISTED) 
  {
    printf("ERROR: Tyring to access black listed webpage or ip\n");
    send(clientfd, accesserror, strlen(accesserror), 0);
    printf("Sent Access Error\n");
    close(clientfd);
    pthread_exit(NULL);
    return NULL;
  }
  else 
  {
		printf("IP Address: %s\n", ipaddr);
		printf("Progress: Not blacklisted\n");
      status = 0;
  }

  /* Get the MD5 hash for the passed URL */
  memset(md5sumofwebpage, '\0', sizeof(md5sumofwebpage));
  uint8_t hmd5 = computemd5hash(md5sumofwebpage, url);
  static uint8_t cacherequirement = ABSENTINCACHE; 
  /* Check to see if the URL is still cached or not */
  uint8_t cachepresencestatus = cachetimeoutcheck(md5sumofwebpage, &cacherequirement);
  printf("Cache Presence Status: %d\n", cachepresencestatus);
  if(cachepresencestatus != FAILED)
  {
   	if(cacherequirement == PRESENTINCACHE)
		{
         printf("PROGRESS: File found in cache. Sending from cache\n"); 
			sendfromcache(clientfd, md5sumofwebpage, temp_buffer);
      }
		else 
		{
			sendfromwebserver(clientfd, webpage, reqsend, &cacherequirement, md5sumofwebpage, extensionfiles, httptype); 
		}
  }
  printf("\n\nMethod: %s\n", requesttype);//client command
  printf("URL: %s\n", url); 
  printf("Webpage: %s\n", webpage); //domain name
  printf("<%lu>:MD5 Hash of Webpage: %s\n", strlen(md5sumofwebpage),md5sumofwebpage);
  printf("HTTP type: %s\n", httptype); //httpversion 
  printf("Port Number: %d\n\n", port);
   printf("IP Address: %s\n", ipaddr); 
  printf("Cache Status: %d\n", cacherequirement);
  printf("Extensions: %s\n", extensionfiles);
  close(clientfd);
  pthread_exit(NULL);

  return NULL;
}


     
  

int receiveclientrequestandstartprocess(int *inclientsocket, int inserversock, struct sockaddr_in *servaddr, socklen_t addrlen)
{
  (*inclientsocket) = accept(inserversock, (struct sockaddr *)servaddr, &addrlen);
  if(inclientsocket < 0) {
    perror("ERROR:accept()\n");
    return -1; 
  }

  pthread_t threadspawned;
  pthread_create(&threadspawned, NULL, respondtoclientrequest, inclientsocket);
  pthread_join(threadspawned, NULL);

  return 0;
}


void logblacklist(void)
{
  FILE *fd;
  fd =fopen("./blacklist", "rb");
  
  if(fd == NULL)
  {
    fprintf(stderr, "ERROR: Failed to open the blacklist file\n");
    exit(0);
  }

  char buffer[1000];
  uint8_t i = 0;

  while(!feof(fd))
  {
    memset(buffer, '\0', sizeof(buffer));
    fgets(buffer,200,fd);
   // printf("\n%s %d\n",buffer,i);
	 strcpy(blacklist.blocked_list[i],buffer);
	// printf("\n%s arraay\n",blacklist.blocked_list[i]);
    i++;    
  }

  blacklistno = i-1;
  
  fclose(fd);

}


int main(int argc, char *argv[])
{
  /* Get the port number for the proxy server from the 
   * command line
   */
  if(argc > 3 || argc <= 1) {
    printf("Please enter valid command line arguments\n./web_proxy <port_num>\n");
    exit(0);
  }

  uint16_t portno = atoi(argv[1]);
  
  if(argv[2] != NULL) {
  timeout = atoi(argv[2]);
  }

  /* Read blacklist file and store in array */
  logblacklist();
  printf("List of Blocked Sites and IPs\n\n"); 
  for(int j =0 ;j<blacklistno; j++)
  {
      printf("%d. %s\n",j,blacklist.blocked_list[j]);
  }
	
  int serversocketfd = 0;
  struct sockaddr_in serveraddress;
  memset(&serveraddress, 0, sizeof(serveraddress));

  /* Initialize the socket values to -1 */
  memset(client_connections, -1, sizeof(client_connections));

  EstablishServer(&serversocketfd, &serveraddress, sizeof(serveraddress), portno);

  socklen_t addrlength = sizeof(serveraddress);

  int16_t activeclients = 0;

  while(1)
  {
     
    receiveclientrequestandstartprocess(&client_connections[activeclients], serversocketfd, &serveraddress, addrlength);

    activeclients = (activeclients+1) % MAX_CONNECTIONS;
    printf("No of Active Client Connections: %d\n", activeclients); 
  }

  close(serversocketfd);

  return 0;
}

