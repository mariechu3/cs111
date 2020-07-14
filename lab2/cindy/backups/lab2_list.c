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
    SortedListElement_t* pool, smallpool;
    int size = numthreads*numit;
    pool = (SortedListElement_t*)malloc(size*sizeof(SortedListElement_t));
    smallpoll = (SortedListElemetn_t*)malloc(size*sizeof(SortedListElement_t));
    char* key = (char*)malloc(size*sizeof(char));
    char** keys = (char**)malloc(size*sizeof(char*));
    for(int i = 0; i < size; i++)
    {
       /* char tmp = ' ' +(char)(rand()%95);
        char tmp2= '\0';
	char temp3 [2];
        temp3[0] = tmp;
        temp3[1] = tmp2;
        */
	key[i] = ' '+(char)(rand()%95);
	keys[i] = &key[i];
        strcat(keys[i], "\0");
	pool[i].key = keys[i];
	//strcat(pool[i].key, "\0");
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
	printf("This is in tmppool %s\n", pool[i].key);
	myargs tmp;
        tmp.mypool = &pool[i*numit];
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
    free(pool);
    free(keys);
    long startn = convert(start.tv_sec, start.tv_nsec);
    long endn = convert(end.tv_sec, end.tv_nsec);
    //char* addyield = "";
    int totalop= numthreads*numit*2;
    long totaltime=endn-startn;
    long avgop = totaltime/totalop;
    printf("list%s%s,%d,%d,%d,%ld,%ld,%lld\n", addyield, addsync,numthreads,numit,totalop,totaltime,avgop,counter);
    
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
    printf("Number of it %d, first in list %s\n", args->it, args->mypool[0].key);
	/*
    int* it = (int*)iterations;
    for(int i=0; i< *it; i++)
    {
	if(syncon)
	{
	    switch(addsync[1])
	    {
	        case 'c':
		    addc(&counter,1);
		    addc(&counter,-1);
		break;
		case 's':
		    adds(&counter,1);
		    adds(&counter,-1);
		break;
		case 'm':
		    addm(&counter,1);
		    addm(&counter,-1);
		break;
            }
	}
        else
	{
	    add(&counter, 1);
            add(&counter, -1);
	}
    } */  
    return (void*)NULL;
}
void signal_handler()
{
    fprintf(stderr, "%s", "Caught Segmentation Fault\n");
}
