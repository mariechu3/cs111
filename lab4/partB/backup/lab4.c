/*NAME: Maree Chu
 * UID: 905116878
 * EMAIL: mariechu@ucla.edu
*/
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#ifdef DUMMY
#define MRAA_GPIO_IN 0
#define MRAA_GPIO_EDGE_RISING 0
typedef int mraa_aio_context;
typedef int mraa_gpio_context;
int mraa_aio_read(mraa_aio_context c)
{
    //this random stuff is just to get rid of warnings for not using the variables
    if(c | !c) 
        return 1000;
    return 1000;
}
void mraa_aio_close(mraa_aio_context c){if(c)c=0;}
int mraa_aio_init(int v){return v;}
int mraa_gpio_init(int v){return v;}
void mraa_gpio_dir(int a, int b){if(a|b)b=0;}
int mraa_gpio_read(int v)
{
    if(v | !v)
       return 0;
    return 0;
}
void mraa_gpio_close(mraa_gpio_context c){if(c)c=0;}
void mraa_gpio_isr(mraa_gpio_context c,int d,void(*e)(void *), void* f)
{
    if(c|d)
	if(e!=NULL&&f!=NULL)
	c=0;
}
#else    
#include <mraa.h>
#include <mraa/aio.h>
#endif
#define B 4275
#define R0 100000.0
void checkdig(char*opt);
float convert(int reading, char* scale);
void options(int argc, char*argv[]);
void printtime(struct tm* tm);
void ps(void* arg);
void parseread(int size);
int period = 1;
char* scale = "F";
int file = 1;
int stop = 0;
int shutdown = 0;
char buf[100];
char readbuf[100];
int size = 0;
int num;
char** arg;
int main(int argc, char* argv[])
{
    num = argc;
    arg = argv;
    struct timespec ts;
    struct tm* tm;
    options(argc,argv);
    //initializes everything
    mraa_aio_context sensor = mraa_aio_init(1);
    mraa_gpio_context button = mraa_gpio_init(62);
    mraa_gpio_dir(button,MRAA_GPIO_IN);
    mraa_gpio_isr(button,MRAA_GPIO_EDGE_RISING,ps,NULL);
    clock_gettime(CLOCK_REALTIME,&ts);
    int temp = 0; 
    int ret = 0;
    char c;
    int cur = 0;
    time_t prev = ts.tv_sec;
    int flags = fcntl(file,F_GETFL,0);
    fcntl(file,F_SETFL, flags|O_NONBLOCK);
    while(true)
    {       
	clock_gettime(CLOCK_REALTIME,&ts);
	tm = localtime(&(ts.tv_sec));
        ret = read(0,&c,1);
	if(ret != -1)
        {
 	    readbuf[cur] = c;
            cur++;
        }
        if(c == '\n')
        {
	    readbuf[cur] = '\0';
	    c = '\0';
	    parseread(cur);
	    cur = 0;
        }
        printf("PREV: %ld, NOW %ld\n",prev, ts.tv_sec);
	if(((int)(ts.tv_sec - prev) == period) && !shutdown)
	{
            prev = ts.tv_sec;
	    write(file,"in the if\n", 10);
	    if(!stop)
	    {
		write(file,"I should print\n", 15);
	        printtime(tm);
	        temp = mraa_aio_read(sensor);
	        size = sprintf(buf,"%f\n",convert(temp,scale));     
 	        write(file,buf,size); 
	    }
	}
        else if (shutdown)
	   break;        
    }
    clock_gettime(CLOCK_REALTIME,&ts);
    printtime(tm);
    write(file,"SHUTDOWN\n",10);
    mraa_gpio_close(button);
    mraa_aio_close(sensor);
}
void parseread(int size)
{
    if(file != 1)
	write(file, readbuf, size);
    char substr[10];
    if(strstr(readbuf,"SCALE=F") != NULL)
    {
	if(strcmp(readbuf,"SCALE=F\n"))
	    fprintf(stderr, "Invalid command %s", readbuf);
	else
	    scale = "F";
    }
    else if(strstr(readbuf, "SCALE=C") != NULL)
    {
        if(strcmp(readbuf,"SCALE=C\n"))
            fprintf(stderr, "Invalid command %s", readbuf);
	else
	    scale = "C";
    }
    else if(strstr(readbuf, "PERIOD=") != NULL)
    {
	int i = 7;
	bool error = false;
	strncpy(substr,readbuf,7);
	substr[7]='\0';
        if(strcmp(substr,"PERIOD="))
            fprintf(stderr, "Invalid command %s, readbuf is: %s", readbuf, substr);
	else
	{
	while(readbuf[i] != '\n')
	{
	    if(!isdigit(readbuf[i]))
            {
		fprintf(stderr, "Invalid command %s", readbuf);
		error = true;
		break;
	    }
            i++;
	}
	if(!error)
        {
	    strncpy(substr,readbuf+7,i-7);
	    substr[i] = '\0';
	    period = atoi(substr);
        }
	}
    }
    else if(strstr(readbuf, "STOP") != NULL)
    {
	if(strcmp(readbuf,"STOP\n"))
            fprintf(stderr, "Invalid command %s", readbuf);
	else
	    stop = 1;
    }
    else if(strstr(readbuf, "START") != NULL)
    {
        if(strcmp(readbuf,"START\n"))
            fprintf(stderr, "Invalid command %s", readbuf);
	else
	    stop = 0;
    }
    else if(strstr(readbuf, "LOG") != NULL)
    {
	strncpy(substr,readbuf,3);
	substr[3] = '\0';	
        if(strcmp(substr,"LOG"))
            fprintf(stderr, "Invalid command %s", readbuf);
    }
    else if(strstr(readbuf, "OFF") != NULL)
    {
	if(strcmp(readbuf,"OFF\n"))
            fprintf(stderr, "Invalid command %s", readbuf);

	else
	    shutdown = 1;
    }
    else
	fprintf(stderr, "Invalid command %s", readbuf);   
     memset(readbuf,0,100);
}

//parses the arguments
void options(int argc, char*argv[])
{
    int c; 
    while(true)
    {
	static struct option long_options[] = 
	{
	    {"period", required_argument, 0, 'p'},
	    {"scale", required_argument, 0, 's'},
	    {"log", required_argument, 0, 'l'},
	};
	int optno = 0;
	c = getopt_long(argc, argv, "", long_options, &optno);
	if(c==-1)
	break;
	switch(c)
	{
	    case 'p':
	    checkdig(optarg);
	    period = atoi(optarg);	    
	    break;
	    case 's':
	    if(strcmp(optarg,"F") != 0 && strcmp(optarg,"C") != 0)
	    {
		fprintf(stderr, "Scale must be 'F' or 'C' not %s\n", optarg);
		exit(1);		
            }
            scale = optarg;
	    break;
	    case 'l':
	    file = open(optarg,O_RDWR|O_CREAT|O_APPEND,0666); //TRUNC?
	    if(file < 0)
	    {
		fprintf(stderr, "Error opening file: %s\n", optarg);
		exit(1);
	    }
	    break;
	}
    }
}
//checks for valid arguments
void checkdig(char* opt)
{
   for(int i =0; i < (int)strlen(opt); i++)
	if(!isdigit(opt[i]))
	{
	    fprintf(stderr, "Invalid argument for period: Must specify a number. \n");
	    exit(1);
	}
}
//called when button is pressed
void ps(void* arg)
{
    shutdown = 1;
    if(arg!=NULL)
	arg=NULL;
}
//converts to temperature
float convert(int reading, char* scale)
{
    float R = 1023.0/((float)reading)-1.0;
    R=R0*R;
    float C = 1.0/(log(R/R0)/B+1/298.15)-273.15;
    float F = (C*9)/5+32;
    if(!strcmp(scale, "C"))
	return C;
    return F;
}
//prints the timestamp
void printtime(struct tm* tm)
{
    size = sprintf(buf,"%02d:%02d:%02d ",tm->tm_hour,tm->tm_min,tm->tm_sec);
    write(file,buf,size);
}
