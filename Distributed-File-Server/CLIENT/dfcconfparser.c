#include<stdio.h>
#include<stdint.h>
#include<string.h> 
#include"doublelinkedlist.h"
#include<stdlib.h>

typedef struct server{
	char ip[20];
	char port[6]; 
} server_t; 

typedef struct serverlist{
  server_t server1; 
  server_t server2;
  server_t server3;
  server_t server4; 
} slist; 

typedef struct credentials{
	char username[10]; 
   char password[10]; 
} cred_t; 

int main()
{
  FILE *fp = fopen("dfc.conf", "r");
  slist servers; 
  cred_t authenticate; 
  if(fp==NULL)
  {
    printf("Error: File Doesnt Exist\n");
  }
  else
  {
    printf("Progress: Config File Parsing Started\n");
    printf("NO of lines: %d\n", lines); 
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
        strcpy(servers.server1.ip, ipad);
        strcpy(servers.server1.port, portno);
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
        strcpy(servers.server2.ip, ipad);
        strcpy(servers.server2.port, portno); 
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
        strcpy(servers.server3.ip, ipad);
        strcpy(servers.server3.port, portno); 
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
        strcpy(servers.server4.ip, ipad);
        strcpy(servers.server4.port, portno);
      }
      if (strncmp(word, "#username", 9) == 0)
      {
        char* value;
        char partsegment[25]; 
        fgets(partsegment, sizeof(partsegment), fp);
        value = strtok(partsegment, "\n"); 
        value = strtok(partsegment, ":"); 
        value = strtok(NULL, ": ");
        strcpy(authenticate.username, value);          
		  
      }
      if (strncmp(word, "#password", 9) == 0)
      {
        char* value;
        char partsegment[25]; 
        fgets(partsegment, sizeof(partsegment), fp);
        value = strtok(partsegment, "\n"); 
        value = strtok(partsegment, ":"); 
        value = strtok(NULL, ": ");
        strcpy(authenticate.password, value);          
		}
     }
      printf(" Server1: %s, %s\n", servers.server1.ip, servers.server1.port); 
      printf(" Server2: %s, %s\n", servers.server2.ip, servers.server2.port); 
      printf(" Server3: %s, %s\n", servers.server3.ip, servers.server3.port); 
      printf(" Server4: %s, %s\n", servers.server4.ip, servers.server4.port); 
      printf(" Username: %s\n", authenticate.username); 
      printf(" Password: %s\n", authenticate.password); 
  } 
  fclose(fp); 
  return 1; 
}
