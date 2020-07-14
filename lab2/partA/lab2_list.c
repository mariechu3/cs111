#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "SortedList.h"
#include <signal.h>

void checkdig(char* name, char*opt); 
void* worker(void* iterations);
long convert(time_t sec, long nsec);
void mul_list_insert(SortedList_t* head, SortedListElement_t* element);
void mul_list_lookup(SortedList_t* head, SortedListElement_t* element);
void mul_list_length(SortedList_t* head);
void mul_list_delete(SortedListElement_t* element);
void spin_list_insert(SortedList_t*head, SortedListElement_t* element);
void spin_list_lookup(SortedList_t*head, SortedListElement_t* element);
void spin_list_length(SortedList_t* head);
void spin_list_delete(SortedListElement_t* element);
long long counter =0;
char addsync[6]= "-none";
char addyield[6] ="-none";
bool syncon = false;
int opt_yield = 0;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
int slock = 0;
void signal_handler();
SortedList_t head;
SortedListElement_t* pool;
int numit=1;
int numthreads=1;
typedef struct
{
    SortedListElement_t* mypool;
    int it;
}myargs;

int main(int argc, char* argv[])
{
    signal(SIGSEGV, signal_handler);
    int c;
    head.next = &head;
    head.prev = &head;
    head.key = NULL;
    while(true)
    {
        static struct option long_options[] =
	{
	    {"threads", optional_argument, 0, 't'},
	    {"iterations", optional_argument, 0, 'i'},
	    {"yield", required_argument, 0, 'y'},
	    {"sync", required_argument, 0, 's'},
	    {0,0,0,0}
	};
	int optno = 0;
	c = getopt_long(argc, argv, "", long_options, &optno);
	if(c==-1)
	    break;
	switch(c)
	{
	    case 't':
	    if(optarg)
	    {
	    	checkdig("--thread", optarg);
                numthreads = atoi(optarg);
	    }
	    break;
	    case 'i':
	    if(optarg)
	    {
		checkdig("--iterations", optarg);
	        numit = atoi(optarg);
            }
	    break;
	    case 'y':
	    if((int)strlen(optarg) > 3)
	    {
		fprintf(stderr, "Too many options of yield, must be 'i', 'd', and/or 'l'\n");
		exit(1);
	    }
            //checks that the arguments to yield are combinations of 'idl'
	    for(int i = 0; i< (int)strlen(optarg); i++)
	    {
		if((char)optarg[i] != 'i' && (char)optarg[i] != 'd' && (char)optarg[i] != 'l')
		{
		    fprintf(stderr, "Invalid option for yield, must be 'i', 'd', and/or 'l'\n");
		    exit(1);
		}
		switch((char)optarg[i])
    		{
		    case 'i':
		    opt_yield |= INSERT_YIELD;
		    break;
		    case 'd':
		    opt_yield |= DELETE_YIELD;
		    break;
		    case 'l':
		    opt_yield |= LOOKUP_YIELD;
		    break;
		}
		addyield[i+1] = (char)optarg[i];
	    }
	    addyield[0] = '-';
	    addyield[(int)strlen(optarg)+1] = '\0';
	    break;
	    case 's':
            //checks that argument to s is either m or s
	    if(strcmp(optarg,"s") && strcmp(optarg,"m"))
	    {
		fprintf(stderr, "--sync only accepts options of 's' or 'm'\n");
		exit(1);
            }
	    syncon = true;
	    addsync[0] = '-';
	    addsync[1] = *optarg;
	    addsync[2] = '\0';
	    break;
	    default:
	    fprintf(stderr, "Invalid option: usage ./lab2_list [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[sm]]\n");
            exit(1);
	    break;
	}
    }
    srand(time(0)); 
    int size = numthreads*numit;
    pool = (SortedListElement_t*)malloc(size*sizeof(SortedListElement_t));
    char** keys = (char**)malloc(size*sizeof(char*));  
    //initiaze keys for the sorted elemented objects to random strings
    for(int i =0; i < size; i++)
    {        
	keys[i] = (char*)malloc(6*sizeof(char));
        for(int j = 0; j < 5; j++)
        {
           keys[i][j] = (char)(' ' +(rand()%95));
        }
        keys[i][5] = '\0';
        pool[i].key = keys[i];
    //    printf("This was put in: %s to this: %s\n", keys[i], pool[i].key);
    }
    //start the clock
    struct timespec start, end;
    if(clock_gettime(CLOCK_MONOTONIC, &start))
    {
	fprintf(stderr, "Error getting starting time.\n");
	exit(1);
    int* threadnum = (int*)malloc(numthreads*sizeof(int));
    //split up the work into separate threads
    for(int i = 0; i <numthreads; i++)
    {
	threadnum[i] = i;
	if(pthread_create(&threads[i], NULL, &worker, (void*)&threadnum[i]))
	{
	    fprintf(stderr, "Error creating thread\n");
	    exit(1);
	}
    }
    for(int i = 0; i < numthreads; i++)
    {
	if(pthread_join(threads[i], NULL))
	{
	    fprintf(stderr, "Error joining threads\n");
	    exit(1);
	}
    }
    //get the final length of the list
    if(SortedList_length(&head))
    {
	fprintf(stderr, "Final length was not 0\n");
        exit(2);
    }
    if(clock_gettime(CLOCK_MONOTONIC, &end))
    {
	fprintf(stderr, "Error getting end time.\n");
	exit(1);
    }
    //free all dynamically allocated variables
    for(int i = 0; i < size; i ++)
        free(keys[i]);
    free(threadnum);
    free(pool);
    free(keys);
    //generate output string
    long startn = convert(start.tv_sec, start.tv_nsec);
    long endn = convert(end.tv_sec, end.tv_nsec);
    int totalop= numthreads*numit*3;
    long totaltime=endn-startn;
    long avgop = totaltime/totalop;
    printf("list%s%s,%d,%d,1,%d,%ld,%ld\n", addyield, addsync,numthreads,numit,totalop,totaltime,avgop);
    
}
//converts the total time in sec and nsec to just nsec
long convert(time_t sec, long nsec)
{
    long result;
    result = 1000000000* ((long)sec);
    result += nsec;
    return result;
}
//checks the arguments of threads and iterations are integers
void checkdig(char* name, char* opt)
{
    for(int i = 0; i < (int)strlen(opt); i++)
         if(!isdigit(opt[i]))
         {
             fprintf(stderr, "Invalid argument for %s: Must specify an number.\n", name);
             exit(1);                   
         }
}
//worker thread to insert/lookup/delete from shared list
void* worker(void* arguments)
{ 
    int start = *((int*)arguments)*numit;
    switch(addsync[1])
    {
	case 'm':
            for(int i = start; i < start+numit; i++)
            {
                int list_num = (int)(
	        pthread_mutex_lock(&mlock);
	        SortedList_insert(&head, &(pool[i]));
                pthread_mutex_unlock(&mlock);
            }
            pthread_mutex_lock(&mlock);
            SortedList_length(&head);
            pthread_mutex_unlock(&mlock);
            for(int i = start; i < start+numit; i++)
            {
                 pthread_mutex_lock(&mlock);
	         SortedListElement_t* temp = SortedList_lookup(&head,pool[i].key);
                 pthread_mutex_unlock(&mlock);
                 if(temp == NULL)
	         {
	             fprintf(stderr,"Error finding element, corrupted list\n");
	             exit(2);		
	         }
                 pthread_mutex_lock(&mlock);
		 int tmp = SortedList_delete(temp);
                 pthread_mutex_unlock(&mlock);
	         if(tmp)
                 {
                     fprintf(stderr,"Error deleting element, corrupted list\n");
	             exit(2);
                 }
            }
	break;
	case 's':
	      for(int i = start; i < start+numit; i++)
              {
                  while(__sync_lock_test_and_set(&slock,1));
                  SortedList_insert(&head, &(pool[i]));
                  __sync_lock_release(&slock);
              }
              while(__sync_lock_test_and_set(&slock,1));
              SortedList_length(&head);
	      __sync_lock_release(&slock);
              for(int i = start; i < start+numit; i++)
              {
		  while(__sync_lock_test_and_set(&slock,1)); 
                  SortedListElement_t* temp = SortedList_lookup(&head,pool[i].key);
                  __sync_lock_release(&slock);
                  if(temp == NULL)
                  {
                       fprintf(stderr,"Error finding element, corrupted list\n");
                       exit(2);
                  }
                  while(__sync_lock_test_and_set(&slock,1));
		  int tmp = SortedList_delete(temp); 
                  __sync_lock_release(&slock);
                  if(tmp)
                  {
                       fprintf(stderr,"Error deleting element, corrupted list\n");
                       exit(2);
                  }
              }
	break;
	default:
             for(int i = start; i < start+numit; i++)
             {
                 SortedList_insert(&head, &(pool[i]));
             }
             SortedList_length(&head);
             for(int i = start; i < start+numit; i++)
             {
                 SortedListElement_t* temp = SortedList_lookup(&head,pool[i].key);
                 if(temp == NULL)
                 {
                     fprintf(stderr,"Error finding element, corrupted list\n");
                     exit(2);
                 }
                 else if(SortedList_delete(temp))
                 {
                     fprintf(stderr,"Error deleting element, corrupted list\n");
                     exit(2);
                 }
             }
        break;	
     }
    return (void*)NULL;
}
void mul_list_insert(SortedList_t* head, SortedListElement_t* element)
{
    int list_num = (int)(element->key) % numlists;
    pthread_mutex_lock(&mlocks[list_num];
    SortedList_insert(&heads[list_num], element);
    pthread_mutex_unlock(&locks[list_num]);
}
SortedListElement_t*  mul_list_lookup(SortedList_t* head, SortedListElement_t* element)
{
    int list_num = (int)(element->key) % numlists;
    pthread_mutex_lock(&mlocks[list_num]);
    SortedListElement_t* temp = SortedList_lookup(&heads[list_num], element); 
    pthread_mutex_unlock(&mlocks[list_num]);
    if(temp == NULL)
    {
	fprintf(stderr, "Error finding element, corrupted list \n");
	exit(2);
    }
    return temp;   
}
void mul_list_length(SortedList_t* head)
{

}
void mul_list_delete(SortedListElement_t* element)
{
    pthread_mutex_lock(&mlocks[list_num]);
    int tmp =  SortedList_delete(&heads[list_num], element);
    pthread_mutex_unlock(&mlocks[list_num]);
    if(tmp)
    {
	fprintf(stderr, "Error deleting element, corrupted list\n");
	exit(2);
    }    
}
void spin_list_insert(SortedList_t*head, SortedListElement_t* element)
{
    int list_num = (int)(element->key) % numlists;
    while(__sync_lock_test_and_set(&slock[list_num]);
    SortedList_insert(&heads[list_num],elements);
    __sync_lock_release(&slock);
}
void spin_list_lookup(SortedList_t*head, SortedListElement_t* element)
{
    int list_num = (int)(element->key) % numlists;
    while(__sync_lock_test_and_set(&slock[list_num]));
    SortedListElement_t* temp = SortedList_lookup(&heads[list_num], element);
    __sync_lock_release(&slock);
    if(temp == NULL)
    {
	fprintf(stderr, "Error finding element, corrupted list \n");
 	exit(2);
    }
}
void spin_list_length(SortedList_t* head)
{
}
void spin_list_delete(SortedListElement_t* element)
{
    while(__sync_lock_test_and_set(&slock[list_num]));
    int tmp = SortedList_delete(&heads[list_num],element);
    __sync_lock_release(&slock);
    if(tmp)
    {
 	fprintf(stderr, "Error deleting element, corrupted list\n");
	exit(2);
    }
}
//catch segmentation faults
void signal_handler()
{
    fprintf(stderr, "Caught Segmentation Fault for thread at address: %p\n", (void*)pthread_self());
    exit(2);
}

