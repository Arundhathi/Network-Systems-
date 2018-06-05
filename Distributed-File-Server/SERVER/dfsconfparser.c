#include<stdio.h>
#include<stdint.h>
#include<string.h> 
#include"doublelinkedlist.h"
#include<stdlib.h>

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


int main()
{
  FILE *fp = fopen("dfs.conf", "r");
  cred_t authenticate[10]; 
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
    printf("No of lines: %d\n", lines);
    int it = 0; 
   while(it < lines)
	{
    while(fgets(parsesegment, sizeof(parsesegment), fp))
    {  
      word = strtok(parsesegment, "\n");
		word = strtok(parsesegment, "\n ");
      strcpy(authenticate[it].username, word);     
      word = strtok(NULL, "\n ");
      strcpy(authenticate[it].password, word);
      printf("%s\n", word);
      it++;  
      
    }
	}
   
     for (it = 0; it< lines; it++)
		{
			printf(" Username:%s\n", authenticate[it].username); 
     		printf(" Password:%s\n", authenticate[it].password);
		} 
}
  fclose(fp); 
  return 1; 
}
