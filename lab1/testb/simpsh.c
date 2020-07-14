/* Name: Marie Chu
 * email: mariechu@ucla.edu
 * UID:905116878
*/

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

extern int optind;
int exitnum = 0;
bool sig = false;
int signalnum = 0;
void signal_handler(int num);
int main(int argc, char* argv[])
{
    if(setvbuf(stdout, NULL, _IONBF, 0))
	fprintf(stderr, "setvbuf failed\n");
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
//	    data[com_num].stind = -1;
//	    data[com_num].endind = -1;
	    data[com_num].childnum = -1;
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
      int comind = 0;
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
      int fddata[fdcount];
      for(int i = 0 ; i < fdcount; i++)
      {
          fddata[i] = -1;
      }
      int fdindex = 0;
      //end store file descriptros
    int fdflags = 0;
    int c;	
    bool verbose = false;
    int waitnum = 0;
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
	{"pause", no_argument, 0, 'y'},       
	{"chdir", required_argument, 0, 'z'},
	{0,0,0,0} 
    };
    int optno = 0;
    c = getopt_long(argc, argv,"", long_options, &optno);
    if(c == -1)
	break;
    switch(c)
    {
	case 'a': //append
	if(verbose)
	    printf("--append\n");
	fdflags |= O_APPEND;
	break;
	case 'b': //cloexec
	if(verbose)
            printf("--cloexec\n");
	fdflags |= O_CLOEXEC;
        break;
	case 'c': //creat
	if(verbose)
            printf("--creat\n");
	fdflags |= O_CREAT;
        break;
	case 'd': //directory
	if(verbose)
            printf("--directory\n");
	fdflags |= O_DIRECTORY;
        break;
	case 'e': //dsync
	if(verbose)
            printf("--dsync\n");
	fdflags |= O_DSYNC;
        break;
	case 'f': //excl
	if(verbose)
            printf("--excl\n");
	fdflags |= O_EXCL;
        break;    
	case 'g': //nofollow
	if(verbose)
            printf("--nofollow\n");
	fdflags |= O_NOFOLLOW;
        break;
	case 'h': //nonblock
	if(verbose)
            printf("--nonblock\n");
	fdflags |= O_NONBLOCK;
        break;
	case 'i': //rsync
	if(verbose)
            printf("--rsync\n");
	fdflags |= O_RSYNC;
        break;
	case 'j': //sync
	if(verbose)
            printf("--sync\n");
	fdflags |= O_SYNC;
        break;
	case 'k': //trunc
	if(verbose)
            printf("--trunc\n");
	fdflags |= O_TRUNC;
        break;
	case 'l': //ronly
	if(verbose)
            printf("--rdonly %s\n", optarg);
        fd = open(optarg, O_RDONLY |fdflags, 0666);
	fdflags = 0;
	if(fd < 0){
	    perror("Error opening file: ");
	    optfail = true;
	}
	fddata[fdindex] = fd;
	fdindex++;
        break;
	case 'm': //rdwr
	if(verbose)
            printf("--rdwr %s\n", optarg);
        fd = open(optarg, O_RDWR | fdflags, 0666);
	fdflags = 0;
	if(fd < 0){
	    perror("Error opening file: ");  
	    optfail = true;
	}
	fddata[fdindex]= fd;
	fdindex++;
        break;
	case 'n': //wronly
	if(verbose)
            printf("--wronly %s\n", optarg);
        fd = open(optarg, O_WRONLY | fdflags, 0666 );
	fdflags = 0;
        if(fd < 0){
	    perror("Error opening file: ");	 
	    optfail = true;
	}
	fddata[fdindex] = fd;
	fdindex++;
        break;
	case 'o': //pipe
	if(verbose)
            printf("--pipe\n");
	int pipefd[2] = {-1,-1};
	if(pipe(pipefd))
	{
	    perror("Error creating pipe: ");
	    optfail = true;
	}
	fddata[fdindex] = pipefd[0];
	fdindex++;
	fddata[fdindex] = pipefd[1];
	fdindex++;
        break;
	case 'p': //command
	if(verbose)
	{
            printf("--command ");
	    for(int i = data[comind].stind; i <= data[comind].endind; i++)
		printf("%s ", copy[i]);
	    printf("\n");    
   	    
	}
	int sz = data[comind].endind-data[comind].stind+1-3+1;
	if( ((atoi(copy[(data[comind].stind)])) > (fdindex - 1)) | ((atoi(copy[(data[comind].stind)+1])) > (fdindex -1)) 
	| ((atoi(copy[(data[comind].stind)+2])) > (fdindex -1)) | fddata[(atoi(copy[(data[comind].stind)]))] == -1 | 
	fddata[(atoi(copy[(data[comind].stind)+1]))] == -1 | fddata[(atoi(copy[(data[comind].stind)+2]))] ==  -1 | sz == 1 )
	{
	    fprintf(stderr, "Error with --command arguments\n");   
	    optfail = true;
	    optind = optind + data[comind].endind - data[comind].stind;
	    comind++;
	    break;
	}
	pid_t rc = fork();
	if(rc == 0)
	{
	    if( dup2(fddata[atoi(copy[(data[comind].stind)])],0)  < 0 )
	       	perror("Error duplicating fd: ");
	    if( dup2(fddata[atoi(copy[(data[comind].stind)+1])],1) < 0)
	        perror("Error duplicating fd: ");
	    if( dup2(fddata[atoi(copy[(data[comind].stind)+2])],2) < 0)
	        perror("Error duplicating fd: ");
	    char *myargs[sz];
	    myargs[sz-1] = NULL;
	    int temp = 0;
	    for(int j = (data[comind].stind)+3; j <= data[comind].endind; j++)
	    {
	       myargs[temp] = copy[j];
	       temp++;
	    }
	    for(int j = 0; j < fdindex ; j++) //closes all files > 2
	    {
	        if(fddata[j] != -1)
	   	  if(close(fddata[j]))
		    perror("Error closing file: ");	
	    }
	    execvp(myargs[0], myargs);		
	    perror("call to execvp failed: ");
	}
	else if(rc > 0)
	    data[comind].childnum = rc;
        else 
	   perror("Error creating child process: ");   
	optind = optind + data[comind].endind - data[comind].stind;
	comind++;
	break;
	case 'q': //wait
	if(verbose)
            printf("--wait\n");
        int status;
	for(int i = waitnum; i < comind; i++)
	{
	    if(data[i].childnum != -1)
	    {
	        if(waitpid(data[i].childnum, &status, 0) < 0)
	        {
                    perror("Error, wait failed");
		    optfail = true;
		    continue;
	        }
		else {
	    	if(WIFEXITED(status))
	    	{		
	    	    if(WEXITSTATUS(status) > exitnum)
		        exitnum = WEXITSTATUS(status);
		    printf("exit %d ", WEXITSTATUS(status));
		    for(int j = data[i].stind+3; j <= data[i].endind; j++)
		            printf("%s ", copy[j]);
		    printf("\n");
	    	} 
	    	if(WIFSIGNALED(status))
	    	{
		    if(WTERMSIG(status) > signalnum)
		    {
		    	sig = true;
		    	signalnum = WTERMSIG(status);
		    }
			printf("signal %d ", WTERMSIG(status));
			for(int j = data[i].stind+3; j<= data[i].endind; j++)
		            printf("%s ", copy[j]);
			printf("\n");
                }
	        }
	    }
	}
	waitnum = comind;
        break;
	case 'r': //close
	if(verbose)
            printf("--close %s\n", optarg);
	if(close(fddata[atoi(optarg)]))
	    perror("Error closing file: ");    	
        fddata[atoi(optarg)] = -1;
	break;
	case 's': //verbose
    	verbose = true;
	break;
	case 't': //profile
	if(verbose)
            printf("--profile\n");
        break;
	case 'u': //abort
	if(verbose)
            printf("--abort\n");
	raise(SIGSEGV);
        break;
	case 'v': //catch
	if(verbose)
            printf("--catch %s\n", optarg);
	signal(atoi(optarg), signal_handler);
        break;
	case 'w': //ignore
	if(verbose)
            printf("--ignore %s\n",optarg);
	signal(atoi(optarg), SIG_IGN);
        break;
	case 'x': //default
	if(verbose)
            printf("--default %s\n", optarg);
	signal(atoi(optarg), SIG_DFL);
        break;
	case 'y': //pause
	if(verbose)
            printf("--pause\n");
	pause();
        break;
	case 'z': //chdir
	if(verbose)
            printf("--chdir %s\n", optarg);
	if(chdir(optarg))
	{
	    optfail = true;
	    perror("Error changing directory: ");
	}
	break;
	default: 
	    fprintf(stderr, "usage: ./simpsh [--append] [--cloexec] [--creat] [directory] ...\n");
	    optfail = true;
    }
    }
    if(optfail && (exitnum < 1))
	exitnum = 1;
    if(sig)
    {
	signal(signalnum, SIG_DFL);
	raise(signalnum);	
    }
    exit(exitnum);
}
void signal_handler(int num)
{
    fprintf(stderr, "%d caught\n", num);  
    if(num > exitnum)
	exitnum = num; 
}
