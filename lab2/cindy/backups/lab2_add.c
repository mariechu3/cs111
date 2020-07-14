#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
void add(long long *pointer, long long value);
void checkdig(char* name, char*opt);
void* worker(void* iterations);
long convert(time_t sec, long nsec);
long long counter =0;
int opt_yield;
int main(int argc, char* argv[])
{
    int c;
    int numthreads;
    int numit;
    char addsync[6] = "-none";
    char* addyield = "";
    bool sync = false;
    while(true)
    {
        static struct option long_options[] =
	{
	    {"threads", optional_argument, 0, 't'},
	    {"iterations", optional_argument, 0, 'i'},
	    {"yield", no_argument, 0, 'y'},
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
	    else
	        numthreads = 1;
	    break;
	    case 'i':
	    if(optarg)
	    {
		checkdig("--iterations", optarg);
	        numit = atoi(optarg);
            }
	    else
		numit = 1;
	    break;
	    case 'y':
	    opt_yield = 1;
	    addyield = "-yield";
  	    break;
	    case 's':
	    if(strcmp(optarg,"s") && strcmp(optarg,"n") && strcmp(optarg,"c"))
	    {
		fprintf(stderr, "--sync only accepts options of 's' or 'n'\n");
		exit(1);
            }
	    sync = true;
	    addsync[0] = '-';
	    addsync[1] = *optarg;
	    addsync[2] = '\0';
	    break;
	    default:
	    fprintf(stderr, "Invalid option: usage ./lab2_add [--threads=#] [--iterations=#] [--yield] [--sync=s/m]\n");
	    break;
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
	if(pthread_create(&threads[i], NULL, &worker, (void*)&numit))
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
    long startn = convert(start.tv_sec, start.tv_nsec);
    long endn = convert(end.tv_sec, end.tv_nsec);
    //char* addyield = "";
    int totalop= numthreads*numit*2;
    long totaltime=endn-startn;
    long avgop = totaltime/totalop;
    printf("add%s%s,%d,%d,%d,%ld,%ld,%lld\n", addyield, addsync,numthreads,numit,totalop,totaltime,avgop,counter);
    
    //if(sync)
	
    //if(yield)
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
void* worker(void* iterations)
{
    int* it = (int*)iterations;
    for(int i=0; i< *it; i++)
    {
	add(&counter, 1);
        add(&counter, -1);
    }   
    return (void*)NULL;
}
void add(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if(opt_yield)
	sched_yield();
    *pointer = sum;
}
