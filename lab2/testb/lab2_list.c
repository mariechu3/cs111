/*NAME: Marie Chu
 *UID: 905116878
 *EMAIL: mariechu@ucla.edu
*/
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

long difference(struct timespec start, struct timespec end);
void checkdig(char* name, char*opt); 
void* worker(void* iterations);
long convert(time_t sec, long nsec);
long long counter =0;
char addsync[6]= "-none";
char addyield[6] ="-none";
bool syncon = false;
int opt_yield = 0;
void signal_handler();
SortedListElement_t* pool;
int numit=1;
int numthreads=1;
int numlists=1;
unsigned long* wait_time;
SortedListElement_t* heads;
pthread_mutex_t* mlocks;
int* slocks; 

int main(int argc, char* argv[])
{
    signal(SIGSEGV, signal_handler);
    int c;
    while(true)
    {
        static struct option long_options[] =
	{
	    {"threads", optional_argument, 0, 't'},
	    {"iterations", optional_argument, 0, 'i'},
	    {"yield", required_argument, 0, 'y'},
	    {"sync", required_argument, 0, 's'},
	    {"lists", required_argument, 0, 'l'},
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
            case 'l':
	    if(optarg)
	    {
		checkdig("--lists", optarg);
                numlists = atoi(optarg);
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
    heads = (SortedList_t*)malloc(numlists*sizeof(SortedList_t));
    slocks = (int*)malloc(numlists*sizeof(int));
    mlocks = (pthread_mutex_t*)malloc(numlists*sizeof(pthread_mutex_t));
    //initialze locks and lists
    for(int i =0; i < numlists;i++)
    {
	heads[i].next = &heads[i];
	heads[i].prev = &heads[i];
	heads[i].key = NULL;
	switch(addsync[1])
    	{
	    case 'm':
	    pthread_mutex_init(&mlocks[i], NULL);
	    break;
	    case 's':
	    slocks[i] = 0;
	    break;
        }
    }
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
    }
    pthread_t threads[numthreads];
    wait_time = (unsigned long*)malloc(numthreads*sizeof(unsigned long));
    int* threadnum = (int*)malloc(numthreads*sizeof(int));
    //split up the work into separate threads
    for(int i = 0; i <numthreads; i++)
    {
	threadnum[i] = i;
        wait_time[i] = 0;
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
    int length = 0;
    for(int i = 0; i < numlists; i++)
        length+=SortedList_length(&heads[i]);
    if(length)
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
    free(slocks);
    free(mlocks);
    free(heads);
    long long totalwait= 0;
    long totallocks = numit*numthreads*(3+numlists);
    for(int i = 0; i < numthreads; i++)
    	totalwait+=wait_time[i];
    free(wait_time);
    //generate output string
    long startn = convert(start.tv_sec, start.tv_nsec);
    long endn = convert(end.tv_sec, end.tv_nsec);
    int totalop= numthreads*numit*3;
    long totaltime=endn-startn;
    long avgop = totaltime/totalop;
    printf("list%s%s,%d,%d,%d,%d,%ld,%ld,%lld\n", addyield, addsync,numthreads,numit,numlists,totalop,totaltime,avgop,totalwait/totallocks);
    
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
long difference(struct timespec start_time, struct timespec end_time)
{
    return convert(end_time.tv_sec, end_time.tv_nsec) - convert(start_time.tv_sec, start_time.tv_nsec);
};
//worker thread to insert/lookup/delete from shared list
void* worker(void* arguments)
{
    struct timespec start_time, end_time; 
    int thread_num = *((int*)arguments);
    int start = thread_num*numit;
    int chance = rand() % 2;
    int length = 0;
    switch(addsync[1])
    {
	case 'm':
            for(int i = start; i < start+numit; i++)
            {
                int list_num = ((int)*(pool[i].key)) % numlists;
		clock_gettime(CLOCK_MONOTONIC, &start_time);
	        pthread_mutex_lock(&mlocks[list_num]);
                clock_gettime(CLOCK_MONOTONIC, &end_time);
                wait_time[thread_num] += difference(start_time,end_time);
	        SortedList_insert(&heads[list_num], &(pool[i]));
                pthread_mutex_unlock(&mlocks[list_num]);
            }
            length = 0;
            if(chance)
                for(int i = 0; i < numlists; i++)
	        {
                    clock_gettime(CLOCK_MONOTONIC, &start_time);
                    pthread_mutex_lock(&mlocks[i]);
                    clock_gettime(CLOCK_MONOTONIC, &end_time);
                    wait_time[thread_num] += difference(start_time,end_time);
                    length += SortedList_length(&heads[i]);
                    pthread_mutex_unlock(&mlocks[i]);
                }
	    else
	        for(int i = numlists-1; i >= 0; i--)
                {
                    clock_gettime(CLOCK_MONOTONIC, &start_time);
                    pthread_mutex_lock(&mlocks[i]);
                    clock_gettime(CLOCK_MONOTONIC, &end_time);
                    wait_time[thread_num] += difference(start_time,end_time);
                    length += SortedList_length(&heads[i]);
                    pthread_mutex_unlock(&mlocks[i]);
                }	
            for(int i = start; i < start+numit; i++)
            {    
                 int list_num = ((int)*(pool[i].key)) % numlists;
                 clock_gettime(CLOCK_MONOTONIC, &start_time);
                 pthread_mutex_lock(&mlocks[list_num]);
                 clock_gettime(CLOCK_MONOTONIC, &end_time);
                 wait_time[thread_num] += difference(start_time, end_time);
	         SortedListElement_t* temp = SortedList_lookup(&heads[list_num],pool[i].key);
                 pthread_mutex_unlock(&mlocks[list_num]);
                 if(temp == NULL)
	         {
	             fprintf(stderr,"Error finding element, corrupted list\n");
	             exit(2);		
	         }
                 clock_gettime(CLOCK_MONOTONIC, &start_time);
                 pthread_mutex_lock(&mlocks[list_num]);
                 clock_gettime(CLOCK_MONOTONIC, &end_time);
                 wait_time[thread_num] += difference(start_time,end_time);
		 int tmp = SortedList_delete(temp);
                 pthread_mutex_unlock(&mlocks[list_num]);
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
                  int list_num = ((int)*(pool[i].key)) % numlists;
                  clock_gettime(CLOCK_MONOTONIC, &start_time);
                  while(__sync_lock_test_and_set(&slocks[list_num],1));
                  clock_gettime(CLOCK_MONOTONIC, &end_time);
                  wait_time[thread_num] += difference(start_time, end_time);
                  SortedList_insert(&heads[list_num], &(pool[i]));
                  __sync_lock_release(&slocks[list_num]);
              }
              length = 0;
              if(chance)
                  for(int i = 0; i < numlists; i++)
                  {
                      clock_gettime(CLOCK_MONOTONIC, &start_time);
                      while(__sync_lock_test_and_set(&slocks[i],1));
                      clock_gettime(CLOCK_MONOTONIC, &end_time);
                      wait_time[thread_num] += difference(start_time,end_time);
                      length += SortedList_length(&heads[i]);
                      __sync_lock_release(&slocks[i]);
                  }
              else
                   for(int i = numlists-1; i >= 0; i--)
                   {
                      clock_gettime(CLOCK_MONOTONIC, &start_time);
                      while(__sync_lock_test_and_set(&slocks[i],1));
                      clock_gettime(CLOCK_MONOTONIC, &end_time);
                      wait_time[thread_num] += difference(start_time,end_time);
                      length += SortedList_length(&heads[i]);
                      __sync_lock_release(&slocks[i]);
                   }
              for(int i = start; i < start+numit; i++)
              {
                  int list_num = ((int)*(pool[i].key)) % numlists;
                  clock_gettime(CLOCK_MONOTONIC, &start_time);
		  while(__sync_lock_test_and_set(&slocks[list_num],1)); 
                  clock_gettime(CLOCK_MONOTONIC, &end_time);
                  wait_time[thread_num] += difference(start_time, end_time);
                  SortedListElement_t* temp = SortedList_lookup(&heads[list_num],pool[i].key);
                  __sync_lock_release(&slocks[list_num]);
                  if(temp == NULL)
                  {
                       fprintf(stderr,"Error finding element, corrupted list\n");
                       exit(2);
                  }
                  clock_gettime(CLOCK_MONOTONIC, &start_time);
                  while(__sync_lock_test_and_set(&slocks[list_num],1));
                  clock_gettime(CLOCK_MONOTONIC, &end_time);
                  wait_time[thread_num] += difference(start_time, end_time);
                  int tmp = SortedList_delete(temp);
                  __sync_lock_release(&slocks[list_num]);
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
                 int list_num = ((int)*(pool[i].key)) % numlists;
                 SortedList_insert(&heads[list_num], &(pool[i]));
             }
             length = 0;
             if(chance)
                 for(int i = 0; i < numlists; i++)
 		     length+=SortedList_length(&heads[i]);
             else
                 for(int i = numlists-1; i >= 0; i--)
                     length+=SortedList_length(&heads[i]);
             for(int i = start; i < start+numit; i++)
             {
                 int list_num = ((int)*(pool[i].key)) % numlists;
                 SortedListElement_t* temp = SortedList_lookup(&heads[list_num],pool[i].key);
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
//catch segmentation faults
void signal_handler()
{
    fprintf(stderr, "Caught Segmentation Fault for thread\n");
    exit(2);
}

