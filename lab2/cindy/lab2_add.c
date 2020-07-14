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
void addm(long long *pointer, long long value);
void addc(long long *pointer, long long value);
void adds(long long *pointer, long long value);
void checkdig(char* name, char*opt);
void* worker(void* iterations);
long convert(time_t sec, long nsec);
long long counter =0;
char addsync[6]= "-none";
bool syncon = false;
int opt_yield;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
int slock = 0;
int main(int argc, char* argv[])
{
    int c;
    int numthreads = 1;
    int numit = 1;
    char* addyield = "";
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
	    break;
	    case 'i':
	    if(optarg)
	    {
		checkdig("--iterations", optarg);
	        numit = atoi(optarg);
            }
	    break;
	    case 'y':
	    opt_yield = 1;
	    addyield = "-yield";
  	    break;
	    case 's':
            //check to make sure the argument is either s, m or c
	    if(strcmp(optarg,"s") && strcmp(optarg,"m") && strcmp(optarg,"c"))
	    {
		fprintf(stderr, "--sync only accepts options of 's' or 'm' or 'c'\n");
		exit(1);
            }
	    syncon = true;
	    addsync[0] = '-';
	    addsync[1] = *optarg;
	    addsync[2] = '\0';
	    break;
	    default:
	    fprintf(stderr, "Invalid option: usage ./lab2_add [--threads=#] [--iterations=#] [--yield] [--sync=[smc]]\n");
	    exit(1);
            break;
	}
    }
    //start time for threads
    struct timespec start, end;
    if(clock_gettime(CLOCK_MONOTONIC, &start))
    {
	fprintf(stderr, "Error getting starting time.\n");
	exit(1);
    }
    //create worker threads
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
    if(clock_gettime(CLOCK_MONOTONIC, &end))
    {
	fprintf(stderr, "Error getting end time.\n");
	exit(1);
    }
    //generate output string
    long startn = convert(start.tv_sec, start.tv_nsec);
    long endn = convert(end.tv_sec, end.tv_nsec);
    int totalop= numthreads*numit*2;
    long totaltime=endn-startn;
    long avgop = totaltime/totalop;
    printf("add%s%s,%d,%d,%d,%ld,%ld,%lld\n", addyield, addsync,numthreads,numit,totalop,totaltime,avgop,counter);
    
}
//convert sec and nanosec to just nanosec
long convert(time_t sec, long nsec)
{
    long result;
    result = 1000000000* ((long)sec);
    result += nsec;
    return result;
}
//check that the arguments to threads and iterations are integers
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
	if(syncon)
	{
            //determine which version of add to use
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
    }   
    return (void*)NULL;
}
//add with mutext locks
void addm(long long *pointer, long long value)
{
    pthread_mutex_lock(&mlock);
    long long sum = *pointer + value;
    if(opt_yield)
        sched_yield();
    *pointer = sum;
    pthread_mutex_unlock(&mlock);
}
//add with compare and swap
void addc(long long *pointer, long long value)
{
    long long prev, sum;
    do{
	prev = *pointer;
	sum = prev+value;
        if(opt_yield)
            sched_yield();
    }while(__sync_val_compare_and_swap(pointer, prev,sum)!= prev);   
}
//add with spin lock
void adds(long long *pointer, long long value)
{
    while(__sync_lock_test_and_set(&slock,1));
    long long sum = *pointer + value;
    if(opt_yield)
        sched_yield();
    *pointer = sum;
    __sync_lock_release(&slock);
}
//regular add
void add(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if(opt_yield)
	sched_yield();
    *pointer = sum;
}
