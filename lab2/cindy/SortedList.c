#include "SortedList.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    SortedList_t *p = list;
    if (opt_yield & INSERT_YIELD)
        sched_yield();
    while (p->next != list) {
        if (p->next->key > element->key) break;
        // printf("|%s|\n", p->next->key);
        fflush(stdout);
        fflush(stdout);
        p = p->next;
    }
    element->next = p->next;
    element->prev = p;
    
    p->next = element;
    element->next->prev = element;
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is pently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete( SortedListElement_t *element) {
    printf("deleting %s\n", element->key);
    if (element->next->prev == element->prev->next) {
        element->prev->next = element->next;
        element->next->prev = element->prev;
        return 0;
    }
    return -1;
    
    
    // SortedListElement_t *prev = element->prev;
    // SortedListElement_t *next = element->next;
    // if ((next->prev != element) || (prev->next != element))
    //     return 1;
    
    // if (opt_yield & DELETE_YIELD)
    //     sched_yield();

    // printf("deleting %s\n", element->key);
    // next->prev = prev;
    // prev->next = next;

    // return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
    SortedListElement_t *p = list->next;

    if (opt_yield & LOOKUP_YIELD)
        sched_yield();

    while (p != list) {
        if (strcmp(p->key, key) == 0) return p;
        if (strcmp(p->key, key) > 1) return NULL;
        p = p->next;
    }
    return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list) {
    int length = 0;
    SortedListElement_t *p = list->next;

    if (opt_yield & LOOKUP_YIELD)
        sched_yield();

    while (p != list) {
        if (!p->prev) return -1;
        p = p->next;
        length++;
    }
    return length;
}