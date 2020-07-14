/*NAME: MARIE CHU
 * UID: 905116878
 * EMAIL: mariechu@ucla.edu
*/

#define _GNU_SOURCE
#include "SortedList.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>

void SortedList_insert(SortedList_t * list, SortedListElement_t *element)
{
    struct SortedListElement* temp = list;
    if(temp->next == list)
    {
        if(opt_yield & INSERT_YIELD)
            sched_yield();
        list->prev = element;
        element->next = list;
        element->prev = list;
        list->next = element;
	return;
    }
    while(temp->next != list)
    {
        temp = temp->next;
        if(strcmp(element->key,temp->key) > 0)
            continue;
	if(opt_yield & INSERT_YIELD)
	    sched_yield();
	element->prev = temp->prev;
	element->next = temp;
	temp->prev->next = element;
	temp->prev = element;
	return;
    }
    if(opt_yield & INSERT_YIELD)
	sched_yield();
    element->prev = temp;
    element->next = list;
    list->prev = element;
    temp->next = element; 
}
int SortedList_delete(SortedListElement_t *element)
{
  if(element->next->prev == element->prev->next)
    {
        element->prev->next = element->next;
        if(opt_yield & DELETE_YIELD)
       	    sched_yield();
        element->next->prev = element->prev;
        return 0;
    }
    return -1;
}
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    struct SortedListElement* temp = list;
    while(temp->next != list)
    {
        temp=temp->next;
        if(opt_yield & LOOKUP_YIELD)
 	    sched_yield();       	
	if(!strcmp(temp->key,key))
	    return temp;
	if(strcmp(temp->key, key) > 0)
	    return NULL;
    }   
    return NULL;
}
int SortedList_length(SortedList_t *list)
{
    int length=0;
    SortedListElement_t* temp = list;
    while(temp->next != list)
    {
	length++;
        if(opt_yield & LOOKUP_YIELD)
	    sched_yield();
        temp = temp->next;
    }
    return length;
}
