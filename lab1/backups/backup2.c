#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char* argv[])
{
    bool optfail = false;
    char substr[3];
    substr[3] = '\0';
    char* copy[argc-1];
    //creating a copy of argv
    for(int i = 1; i < argc;i++)
    {
	copy[i-1]=argv[i];
    }
    int fd;
    int c;
    //storing command option
     char command[10];
        int num_com = 0;
        command[9] = '\0';
        struct cinfo
        {
            pid_t childnum;
            int stind;
            int endind;
        };
        for(int i = 0; i < argc-1; i++)
        {
            strncpy(command, copy[i], 9);
	    command[9] = '\0';
            if(strcmp(command,"--command") == 0)
                num_com++;
        }
        struct cinfo data[num_com];
        int com_num = 0;
        for(int i = 0; i < argc-1; i++)
        {
            strncpy(command, copy[i], 9);
	    command[9] = '\0';
            if(strcmp(command, "--command")==0)
            {
                data[com_num].stind = i+1;
                for(int j = i+1; j < argc-1; j++)
                {
                    strncpy(substr, copy[j], 2);
		    substr[2] = '\0';
                    if(strcmp(substr, "--") == 0)
                    {
                        data[com_num].endind = j-1;
                        break;
                    }
		    if(j == argc-2)
			data[com_num].endind = j;
                }
                com_num++;
            }
        }
	//end of storing command
	//storing file descriptors
	int fdcount = 0;
	for(int i = 0; i < argc-1; i++)
	{
	    if(strcmp(copy[i], "--rdonly") == 0 | strcmp(copy[i], "--rdwr") ==0 |strcmp(copy[i], "--wronly") == 0)
		fdcount++;
	    if(strcmp(copy[i], "--pipe") == 0)
		fdcount +=2;
	}
//	struct fdmap fddata[fdcount];
	int fddata[fdcount];
	for(int i = 0 ; i < fdcount; i++)
	{
	    fddata[i] = -1;
	}
	int fdindex = 0;
	//end store file descriptros
    while(true)
    {
    static struct option long_options[] = 
    {
	{"append", no_argument, 0, 'a'},
	{"cloexec", no_argument, 0, 'b'},
	{"creat", no_argument, 0, 'c'},
	{"directory", no_argument, 0, 'd'},
	{"dsync", no_argument, 0, 'e'},
	{"excl", no_argument, 0, 'f'},
	{"nofollow", no_argument, 0, 'g'},
	{"nonblock", no_argument, 0, 'h'},
	{"rsync", no_argument, 0, 'i'},
	{"sync", no_argument, 0, 'j'},
	{"trunc", no_argument, 0, 'k'},
	{"rdonly", required_argument, 0, 'l'},
	{"rdwr", required_argument, 0, 'm'},
	{"wronly", required_argument, 0, 'n'},
	{"pipe", no_argument, 0, 'o'},
	{"command", required_argument, 0, 'p'},
	{"wait", no_argument, 0, 'q'},
	{"close", required_argument, 0, 'r'},
	{"verbose", no_argument, 0, 's'},
	{"profile", no_argument, 0, 't'},
	{"abort", no_argument, 0, 'u'},
	{"catch", required_argument, 0, 'v'},
	{"ignore", required_argument, 0, 'w'},
	{"default", required_argument, 0, 'x'},
	{"pause", no_argument, 0, 'y'}        
    };
    int optno = 0;
    c = getopt_long(argc, argv,"", long_options, &optno);
    if(c == -1)
	break;
    switch(c)
    {
	case 'a': //append
	break;
	case 'b': //cloexec
        break;
	case 'c': //creat
        break;
	case 'd': //directory
        break;
	case 'e': //dsync
        break;
	case 'f': //excl
        break;    
	case 'g': //nofollow
        break;
	case 'h': //nonblock
        break;
	case 'i': //rsync
        break;
	case 'j': //sync
        break;
	case 'k': //trunc
        break;
	case 'l': //ronly
        fd = open(optarg, O_RDONLY);
	if(fd < 0){
	    fprintf(stderr, "Error opening file: %s\n", optarg);
	    optfail = true;
	}
	fddata[fdindex] = fd;
	fdindex++;
        break;
	case 'm': //rdwr
        fd = open(optarg, O_RDWR);
	if(fd < 0){
	    fprintf(stderr, "Error openning file: %s\n", optarg);
	    optfail = true;
	}
	fddata[fdindex]= fd;
	fdindex++;
        break;
	case 'n': //wronly
        fd = open(optarg, O_WRONLY);
        if(fd < 0){
	    fprintf(stderr, "Error opening file.%s\n", optarg);
	    optfail = true;
	}
	fddata[fdindex] = fd;
	fdindex++;
        break;
	case 'o': //pipe
        break;
	case 'p': //command
	;int retval;
	for(int i = 0 ; i < num_com; i++)
	{
	    if(atoi(copy[(data[i].stind)]) > fdindex | atoi(copy[(data[i].stind)+1]) > fdindex | atoi(copy[(data[i].stind)+1]) > fdindex)
	    {
		optfail = true;
		continue;
            }
	    pid_t rc = fork();
	    if(rc == 0)
	    {
		retval = dup2(fddata[atoi(copy[(data[i].stind)])],0);
//		printf("File descriptor: %d\n", fddata[atoi(copy[(data[i].stind)])]);
		if(retval < 0)
		{
		    fprintf(stderr, "Error duplicating to file descriptor 0\n");
		    optfail = true;	    
		}
		retval = dup2(fddata[atoi(copy[(data[i].stind)+1])],1);
//		printf("File descriptor: %d\n", fddata[atoi(copy[(data[i].stind)+1])]);
		if(retval < 0)
		{
		    fprintf(stderr, "Error duplicating to file descriptor 1\n");
		    optfail = true;
		}
		retval = dup2(fddata[atoi(copy[(data[i].stind)+2])],2);
//		printf("File descriptor: %d\n", fddata[atoi(copy[(data[i].stind)+2])],2);
		if(retval < 0)
		{
		    fprintf(stderr,"Error duplicating to file descriptor 2\n");
		    optfail = true;
		}
		if(!optfail)
		{
		int sz = data[i].endind-data[i].stind+1-3+1;
		char *myargs[sz];
		myargs[sz-1] = NULL;
		int temp = 0;
		for(int j = (data[i].stind)+3; j <= data[i].endind; j++)
		{
		   myargs[temp] = copy[j];
		   temp++;
		}
		for(int j = 3; ; j++) //closes all files > 2
		{
		    if(close(j))
			break;
		}
		execvp(myargs[0], myargs);		
		}
	    }
	    else if(rc > 0)
	    {
		data[i].childnum = rc;
            }
	    else 
	    	fprintf(stderr, "Fork unsuccessful");
	}
        break;
	case 'q': //wait
        break;
	case 'r': //close
        break;
	case 's': //verbose
        ;int index;
        for(int i = 0; i < argc-1; i++)
        {
	    //printf("at index %d: %s\n", i, copy[i]);
	    if(strcmp(copy[i],"--verbose") == 0)
		index = i;
        }
        bool first = true;
	for(int i = index+1; i < argc-1; i++)
	{
            strncpy(substr, copy[i], 2);
	    substr[2] = '\0';
            if((strcmp(substr, "--") == 0) && !first)
           	printf("\n");
            if(first)
		first = false;
	    printf("%s ", copy[i]);
	}
	break;
	case 't': //profile
        break;
	case 'u': //abort
        break;
	case 'v': //catch
        break;
	case 'w': //ignore
        break;
	case 'x': //default
        break;
	case 'y': //pause
        break;
	default: 
	    fprintf(stderr, "usage: ./simpsh [--append] [--cloexec] [--creat] [directory] ...\n");
	    optfail = true;
    }
    }
    if(optfail)
	exit(1);
}
