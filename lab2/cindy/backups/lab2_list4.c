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
long long counter =0;
char addsync[6]= "-none";
char addyield[6] ="-none";
bool syncon = false;
int opt_yield;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
int slock = 0;
void signal_handler();
SortedList_t head;
typedef struct
{
    SortedListElement_t* mypool;
    int it;
}myargs;

int main(int argc, char* argv[])
{
    signal(SIGSEGV, signal_handler);
    int c;
    int numthreads = 1;
    int numit = 1;
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
	    for(int i = 0; i< (int)strlen(optarg); i++)
	    {
		if((char)optarg[i] != 'i' && (char)optarg[i] != 'd' && (char)optarg[i] != 'l')
		{
		    fprintf(stderr, "Invalid option for yield, must be 'i', 'd', and/or 'l'\n");
		    exit(1);
		}
		addyield[i+1] = (char)optarg[i];
	    }
	    addyield[0] = '-';
	    addyield[(int)strlen(optarg)+1] = '\0';
	    opt_yield = 1;
  	    break;
	    case 's':
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
	    break;
	}
    }/*
    typedef struct
    {
        SortedListElement_t mypool[numit];
        int it;
    }myargs;
*/
    SortedListElement_t** pool;
    int size = numthreads*numit;
    pool = (SortedListElement_t**)malloc(numthreads*sizeof(SortedListElement_t*));
    //char* key = (char*)malloc(5*sizeof(char));
    char** keys = (char**)malloc(size*sizeof(char*));
    for(int i = 0 ; i < numthreads; i++)
    {
	pool[i] = (SortedListElement_t*)malloc(numit*sizeof(SortedListElement_t));
    }  
    for(int i =0; i < size; i++)
    {        
	keys[i] = (char*)malloc(5*sizeof(char));
        for(int j = 0; j < 4; j++)
        {
           keys[i][j] = (char)(' ' +(rand()%95));
        }
        keys[i][4] = '\0';
        printf("This was put in: %s\n", keys[i]);
    }
    for(int i = 0; i < numthreads; i++)
    {
	for(int j =0; j < numit; j++)
	{
	   int num = (i*numit)+j;
	   pool[i][j].key = keys[num];	
           printf("Now it is this: %s\n", pool[i][j].key);
	}
    }
    struct timespec start, end;
    if(clock_gettime(CLOCK_MONOTONIC, &start))
    {
	fprintf(stderr, "Error getting starting time.\n");
	exit(1);
    }
    pthread_t threads[numthreads];
    for(int i = 0; i <numthreads; i++)
    {
	//printf("This is in tmppool %s\n");
	myargs tmp;
        tmp.mypool = pool[i];
	tmp.it = numit;
	if(pthread_create(&threads[i], NULL, &worker, (void*)&tmp))
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
    //printf("Counter is %lld\n", counter);
    if(clock_gettime(CLOCK_MONOTONIC, &end))
    {
	fprintf(stderr, "Error getting end time.\n");
	exit(1);
    }
    for(int i = 0; i < numthreads; i++)
	free(pool[i]);
    free(pool);
    free(keys);
    long startn = convert(start.tv_sec, start.tv_nsec);
    long endn = convert(end.tv_sec, end.tv_nsec);
    //char* addyield = "";
    int totalop= numthreads*numit*3;
    long totaltime=endn-startn;
    long avgop = totaltime/totalop;
    printf("list%s%s,%d,%d,1,%d,%ld,%ld\n", addyield, addsync,numthreads,numit,totalop,totaltime,avgop);
    
}
long convert(time_t sec, long nsec)
{
    long result;
    result = 1000000000* ((long)sec);
    result += nsec;
    return result;
}
void checkdig(char* name, char* opt)
{
    for(int i = 0; i < (int)strlen(opt); i++)
         if(!isdigit(opt[i]))
         {
             fprintf(stderr, "Invalid argument for %s: Must specify an number.\n", name);
             exit(1);                   
         }
}
void* worker(void* arguments)
{
    myargs* args = (myargs*)arguments; 
    for(int i = 0; i < args->it; i++)
    {
    	printf("Finally in list %s\n", args->mypool[i].key);
    }
    for(int i = 0; i < args->it; i++)
    {
	SortedList_insert(&head, &(args->mypool[i]));
    }
    printf("This is the length: %d\n", SortedList_length(&head));
    //sleep(1);
    for(int i = 0; i <args->it; i++)
    {
	SortedList_delete(SortedList_lookup(&head, args->mypool[i].key));
    }
    printf("This is the length: %d\n", SortedList_length(&head));
    return (void*)NULL;
}
void signal_handler()
{
    fprintf(stderr, "%s", "Caught Segmentation Fault\n");
    exit(2);
}

