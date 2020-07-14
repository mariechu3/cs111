#include "SortedList.h"
void SortedList_insert(SortedList_t * list, SortedListElement_t *element)
{
    struct SortedListElement* temp = list;
    if(list->prev = list)
    {
        list->prev = element;
        element->next = list;
    }
    else
    {
        while(temp->next != list)
        {
	    if(temp->key < temp->next->key)
		temp = temp->next;
	    else
		break;
        }
	
	/*
	list->prev->next = element;
        element->prev = list->prev;
        list->prev = element;
	element->next = list;
	*/
    }
    if(list->next = list)
    {
	list->next = element;
	element->prev = list;
    }   
}
int SortedList_delete(SortedListElement_t *element)
{
    element->prev->next = element->next;
    element->next->prev = element->prev;
    free(element); //syntax
}
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t* temp = list;
    while(temp->next != list)
    {
        if(temp->key == key)
	    return temp;   
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
        temp = temp->next;
    }
    return length;
}
