/*
#CODE: Doubly Linked List - Basic Functions
#LANGUAGE: C
#AUTHOR: Arundhathi Swami
#COURSE/PROJECT: Advanced Practical Embedded Systems
#DATE STARTED: Sept 3 2017
#LAST DATE MODIFIED: Sept 5 2017
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "doublelinkedlist.h"
#include <string.h>
/*Input Parameters: None
  Return: struct LINKEDLISTNODE* (pointer to created node)
  Function Description: Dynamically allots space and address to linked list
*/
node* create()
{
	return ((node*)malloc(sizeof(node))); //returns typecasted pointer pointing to address of allocated space
}


/* Input Parameters: struct LINKEDLISTNODE* - to headref
   Return: size_t - length of linked list
   Function Description: Returns the number of nodes or length of the linked list
*/
size_t size(node* headref)
{
    node* current = headref;
    size_t size_of_list = 0;

	if ( headref == NULL ) //checks for empty list
    {
        return size_of_list;
    }
	else
    {
        while(current != NULL) //waits for end of list
		{

			size_of_list += 1;
			current = current-> next; //traverses list

        }
		return (size_of_list);
	}

}


/* Input Parameters: struct LINKEDLISTNODE** - to headref (starting node of linked list), uint32_t data - data to be allocated to new node, uint32 position - index at which to add node
   Return: status - errors/success resulting from various operations
   Function Description: Destroys all the nodes in a linked list
*/
status destroy(node** headref)
{
    size_t length = size(*headref);
    size_t here;
    node* current = *headref;
    node* destroy = current;
    if (length == 0)
    {
        return list_empty;

    }
	else if (current == NULL)
	{
		return failure;
	}
    else
    {
        for (here = 0; here<length; here++)
        {
            destroy = current;
            current = current->next;
            free(destroy);
        }
        *headref = NULL;
        return success;
    }

}

/* Input Parameters: struct LINKEDLISTNODE** - to headref (starting node of linked list), uint32_t data - data to be allocated to new node, uint32 position - index at which to add node
   Return: status - errors/success resulting from various operations
   Function Description: Adds Node at a specified position in the list
   */
status add_node(node** headref, char* new_function, char* new_value, uint32_t position)
{

   size_t length = size(*headref); //obtains current length of the linked list
	
    uint32_t position_in_list = 0;
    node* current = *headref; //ref node pointing to head ref node
    node* before = *headref;

    /*Creates new node to enlisted in the linked list*/
    node* enlist = create();
    strcpy(enlist->function, new_function);
    strcpy(enlist->value, new_value);
    //enlist->function = new_function;
    //enlist->value = new_value;
 //   printf("Function: %s\n", enlist->function);
   // printf("Value: %s\n", enlist->value);
    /*corner case: No list exists*/
    if (*headref == NULL)
    {
        *headref = enlist; //allocates new node as headref
        enlist->prev = NULL;
        enlist->next=NULL;
        return node_added;
    }

    else
    {
        /*corner case: Passed index parameter is greater than size of linked list*/
        if (position > length)
        {
            return out_of_range;
        }
        /*corner case: Passed index parameter is top/first node of the linked list*/
        else if (position == 0)
        {
            enlist->next = *headref;
            enlist->prev = NULL;
            current->prev = enlist;
            *headref = enlist;
            return node_added;
        }
        /*corner case: Passes index parameter is last/after the last node of the linked list*/
        else if (position == length)
        {
            while (current->next != NULL)
            {
                current = current->next;
            }

            enlist->prev =current;
            enlist->next = NULL;
            current->next = enlist;
            return node_added;
        }
        /*Passed index parameter is between headref node and last node*/
        else
        {
            while (current != NULL)
            {

                position_in_list += 1;
                before = current;
                current = current->next;
                if (position_in_list == position)
                {
                    enlist->next = current;
                    enlist->prev = current->prev;
                    current->prev = enlist;
                    before->next = enlist;
                    return node_added;
                }

            }
        }

    }
	return failure;
}


/* Input Parameters: struct LINKEDLISTNODE** - to headref (starting node of linked list), uint32 position - index of node to be removed
   Return: status - errors/success resulting from various operations
   Function Description: Removes node in specified position
*/
status remove_node(node** headref, uint32_t node_position, char** new_function, char** new_value)
{
    uint32_t to_remove = 0;
    size_t length = size(*headref);
    node* current = *headref; //node* pointing to headref node
   // node* before = *headref;

    /*corner case: no linked list exists or node to be removed is not part of the linked list*/
    if (current == NULL || node_position>length)
    {
        return out_of_range;
    }
    /*corner case: parameter passed as node to be removed is the first node/headref*/
    if (node_position == 0)
    {
        *new_function = current->function;
        *new_value = current->value;
        current->next->prev = NULL;
        *headref = current->next; //assigns second node as headref / 0th node
        free(current);
        return node_removed;
    }
    /*corner case: parameter passed as node to be removed is at the end of the linked list*/
    if (node_position == (length))
    {
        while(current->next!=NULL)
        {
            current=current->next; //navigates to the last node
        }
        *new_function = current->function;
        *new_value = current->value;
        current->prev->next = NULL; //allocates second-to-last node as last
        free(current);
        return node_removed;
    }
    else
    {
        while (to_remove != node_position)
        {
            current = current->next; //navigates to required index
            to_remove += 1;
        }
        *new_function = current->function;
        *new_value = current->value;
        current->prev->next = current->next;
        current->next->prev = current->prev;
        free(current);
        return node_removed;
    }

}

/* Input Parameters: struct LINKEDLISTNODE** - to headref (starting node of linked list), uint32_t search_for - data to be searched for, uint32_t* index - used to pass index of found node back to function
   Return: node* node - returns a pointer to the node containing the required data
   Function Description: Searches for a specified data from within the linked list
   NOTE: if data is not found, it returns a pointer to the headref
   */
int search_node(node* headref, char* search_for)
{
    node* current = headref;
    if (current == NULL)
    {
      printf("Heyo\n"); 
    }
    uint32_t position = 0;
    while (current != NULL)
    {
        if (strcmp(search_for, current->value) == 0)
        {
            return 1;
        }
        
        current = current->next; //traverses list

    }
    return 0; //returns default headref pointer if required data isnt found
}

/* Input Parameters: struct LINKEDLISTNODE** - to headref (starting node of linked list)
   Return: status - errors/success resulting from various operations
   Function Description: Prints out a linked list
*/
status print_list (node** headref)
{
    uint32_t length = size(*headref);
   // printf("length = %d", length);

    if (length == 0)
    {
        return list_empty;
    }
    node* current = *headref;
    while (current != NULL)
    {
        printf("Description: %s\n ", current->function); //prints out corresponding element
        printf(" Extension: %s\n", current->value); //prints out corresponding element

        current = current->next; //traverses the list
    }
    printf("Length of the List is: %d\n" , length); //prints final length of the list
    return success;

}




