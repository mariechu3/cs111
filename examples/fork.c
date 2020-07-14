#include <stdio.h>
#include <unistd.h>

int main() 
{
printf("a\n");
pid_t child = fork();
if(child > 0)
{
	sleep(1);
	printf("b\n");
        
}
else if (child == 0)
	printf("c\n");
else
	printf("d\n");
}
