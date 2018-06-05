#include<stdio.h>
#include<stdint.h>
#include<string.h> 
#include"doublelinkedlist.h"
#include<stdlib.h>

typedef struct webpagelist{
  char* page1; 
  char* page2;
  char* page3;
} wlist; 


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

int parsePortNumber(char* line)
{
  printf("Progress: Getting Port Number\n"); 
  char* portno; 
  portno = strtok(line, " "); 
  portno = strtok(NULL, ""); 
  return atoi(portno); 
}

char* parseDocumentRoot(char* line)
{
  printf("Progress: Getting Document Root\n"); 
  char* rootpwd; 
  rootpwd = strtok(line, " ");
  rootpwd = strtok(NULL, "\"");
  return rootpwd; 
}

wlist parseDefaultWebPage(char* line)
{
  printf("Progress: Getting Default Web Pages\n"); 
  char* webpgs;
  wlist list; 
  webpgs = strtok(line, " "); 
  webpgs = strtok(NULL, " ");
  list.page1 = webpgs;
  webpgs = strtok(NULL, " "); 
  list.page2 = webpgs; 
  webpgs = strtok(NULL, " ");
  list.page3 = webpgs;
  return list; 
}

int parseTimeout(char* line)
{
  char* timeout; 
  timeout = strtok(line, " "); 
  timeout = strtok(NULL, " "); 
  timeout = strtok(NULL, ""); 
  int time = atoi(timeout); 
  return time;
}


int main()
{
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
    node* headRef = NULL; 
    
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
        int port = parsePortNumber(value); 
        printf("Port No: %d\n", port); 
      }
      else if (strncmp(word, "#document root", 14) == 0)
      {
        char* value;
        char partsegment[70]; 
        fgets(partsegment, sizeof(partsegment), fp);
        value = strtok(partsegment, "\n");
        char* root;
        root = parseDocumentRoot(value);
        printf("Document Root: %s\n", root); 
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
        node* headRef = NULL;
        char* extension; 
        char* description;
        while(iteration<9)
        {
          fgets(partsegment, sizeof(partsegment), fp);
          value = strtok(partsegment, "\n");
          extension = strtok(value, " "); 
          description = strtok(NULL, ""); 
          int ret = add_node(&headRef, &description, &extension, 1);  
          //printf("%d\n", headRef);
          memset(partsegment, '\0', sizeof(partsegment));
          iteration++;
        }
          
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

    
    }

  }
  return 0; 
}
