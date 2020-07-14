#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
if(close (1))
	printf("success");

}
