#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<getopt.h>
#include<stdbool.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
//declaration of functions
void invalid();
void signal_handler();
void seg();
int main(int argc, char* argv[])
{
  int c;
  bool input_flag = false, output_flag = false, segfault_flag = false, catch_flag = false, dumpcore_flag = false;
  char* input_file, *output_file;

  //parsing arguments
  while(true)
  {
    static struct option long_options[] =
    { 
       {"input",  required_argument, 0, 'i'},
       {"output", required_argument, 0, 'o'},
       {"segfault",     no_argument, 0, 's'},
       {"catch",        no_argument, 0, 'c'},
       {"dump-core",    no_argument, 0, 'd'},
    };
    int optno = 0;
    c = getopt_long (argc, argv, "", long_options, &optno);
    if (c == -1)
       break;
    switch (c)
    {
       case 'i':
       input_flag = true;
       //printf("Input\n");
       input_file = optarg;
       break;
       case 'o':
       output_flag = true;
       output_file = optarg;
       //printf("Output \n");
       break;
       case 's':
       segfault_flag = true;
       break;
       case 'c':
       catch_flag = true;
       //printf("catchflag \n");
       break;
       case 'd':
       dumpcore_flag = true;
       //printf("dumpcore \n");
       break;
       default:
       invalid();
       break; 
    }
  }
  //doing the correct action for arguments that are parsed
 int src_fd = 0;
 int dst_fd = 1;
 if(segfault_flag)
 {
    if(catch_flag)
        signal(SIGSEGV, signal_handler);
    seg();
 }
 if(input_flag)
 {  
     src_fd = open(input_file, O_RDONLY);
     if(src_fd < 0){
       fprintf(stderr, "error opening %s \nExit with code 2 \n", input_file);      
       exit(2);
     } 
 } 
 if(output_flag)
 {
     dst_fd = open(output_file, O_WRONLY);
     if(dst_fd < 0){
       fprintf(stderr, "error opening %s \nExit with code 3 \n", output_file);
       exit(3);
     }
 }
char buf;
while(read(src_fd,&c,1) > 0)
{
   write(dst_fd,&c,1);
}     
}



void signal_handler()
{
   fprintf(stderr, "%s", "Caught segmentation fault\nExit with code 4\n");
   exit(4);
}
void seg()
{
   int* nothing = NULL;
   *nothing = 0;  
}
void invalid(char* arg)
{
   fprintf(stderr, "%s is an invalid argument\n", arg);
   exit(1);
}
