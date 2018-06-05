#Readme for:
#CODE: UDP Protocol - Server Code 
#LANGUAGE: C
#AUTHOR: Arundhathi Swami
#COURSE/PROJECT: Advanced Practical Embedded Systems
#DATE STARTED: Sept 12 2017
#LAST DATE MODIFIED: Sept 24 2017



Files and Folders: 
a. Server - In main folder. Compiled using makefile. 
b. Client - In "client" folder. Compiled using Makefile. 

Commands: 
1. To start server: ./server port_no 
2. To start client: ./client ip port_no

Menu: 
On client end, you can access the following commands: 
a. get filename
This command requests the remote server for a file. The server initiates transmission of this file. 
If the filename is incorrect, it exits the command and you can try again. 

b. put filename
This command informs the remote server that it will transmit a file to the server and initaites transmission. 
If the filename us invalid, it exits the command and you can try again. 

c. ls
This command requests the server to send a list of files available in its root directory. 

d. delete filename
This command requests the remote server to delete a file from its root directroy. 
Invalid filename produces an error and asks to try again. 

e. exit 
This file closes the socket connection. 


Reliability Implementation: 
Reliablity mechanism has been implemented using two methods: 
1. A number that identifies the packet number. This number is identified as "reliability_bit". It alternates between 0 and 1 for consecutive packets. 
This number is then sent back as confirmation from the receivers end. If the relibility and confirmation match, the packets have arrived in the right order. 
2. Timeout- A stop-and-wait type of mechanism has been used. The receiver or sender times out and resends if the appropriate packet number or confirmation hasnt been received in time. 

Encryption Implementation: 
An exor encryption has been applied. Each char of every packet is exored with "1". This effectively "nots" the value of the bit. Unless thes packets are exored again at the receiver end, the file is unreadble. 
