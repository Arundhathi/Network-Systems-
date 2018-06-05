/*
#CODE: Doubly Linked List Header File 
#LANGUAGE: C
#AUTHOR: Arundhathi Swami
#COURSE/PROJECT: Advanced Practical Embedded Systems
#DATE STARTED: Sept 3 2017
#LAST DATE MODIFIED: Sept 5 2017
#*/

#ifndef DOUBLELINKEDLIST_H_INCLUDED
#define DOUBLELINKEDLIST_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

/*
Data Structure representing one node constituent of Linked List
Members:
1. data: data to be allocated to a particular node
2. struct LINKEDLISTNODE* next : pointer to the next linked list node
3. struct LINKEDLISTNODE* prev : pointer to the previous linked list node
*/
typedef struct LINKEDLISTNODE {
	char function[20];
   char value[20]; 
	struct LINKEDLISTNODE* next;
	struct LINKEDLISTNODE* prev;

	} node;
/*
Enum enumerating errors occuring during various processes in the file
Members:
*/
typedef enum OPERATION_ERRORS {
	success = 1,
	failure = 2,
	out_of_range = 3,
	node_added = 4,
	node_removed = 5,
	node_not_present = 6,
	node_not_found = 7,
	node_found = 8,
	list_empty = 9,
	} status;

/*List of all functions*/ 

/* Function Description: Dynamically allots space and address to linked list*/
node* create();

/*Function Description: Returns the number of nodes or length of the linked list*/
size_t size(node* headref);

/*Function Description: Destroys all the nodes in a linked list*/
status destroy(node** headref);

/*Function Description: Adds Node at a specified position in the list*/
status add_node(node** headref, char* new_function, char* new_value, uint32_t position);

/*Function Description: Removes node in specified position*/
status remove_node(node** headref, uint32_t node_position, char** new_function, char** new_value);

/*Function Description: Searches for a specified data from within the linked list
   NOTE: if data is not found, it returns a pointer to the headref*/
int search_node(node* headref,char* search_for);

/*Function Description: Prints out a linked list*/
status print_list (node** headref);



















#endif // DOUBLELINKEDLIST_H_INCLUDED
