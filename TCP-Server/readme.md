#Readme for:
#CODE: HTTP web server 
#LANGUAGE: C
#AUTHOR: Arundhathi Swami
#COURSE/PROJECT: Network Systems 
#DATE STARTED: OCT 13 2017
#LAST DATE MODIFIED: OCT 22 2017

Files and Folders: 
a. tcp_webserver - folder contains make file for webserver. Source file for server is httpservertest.c. Included within the folder is doublelinkedlist.c to help with linked list functionlity. 
b. www - contains all the files included on the webpage
c. ws.conf - contains attributes essential to the server configuration. Server configuration can be modified by modifying the ws.conf file 


Commands: 
1. To start the webserver: navigate to tcp_webserver from CLI and enter make
2. To start server on browser: navigate address bar to: localhost:9990
3. You can change the port numbers in the conf file 

Menu: 
Once the webpage is navigated, it will display a list of files that are available
Each of the files accessed will result in GET requests to the server. 


To implement a POST command enter the following command: (echo -en "POST /files/text1.text HTTP/1.1\nHost: localhost\nConnection: Keepalive\n\nPOSTDATA";sleep 10) | telnet 127.0.0.1 9990

the part after POST should contain the relative path to the file that needs to be posted. 
the part after telnet should contain the local host ip and port number being used

The  output will be displayed on the command line from which the POST request was sent as well as on the command line on which the server is running. 
