#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
   char tmp = 'a';
   char tmp2 [10];
   strcpy(tmp2, &tmp);
   strcat(tmp2, "\0");
   //strcat(tmp2,"hello\0");
   printf("hello?:%s", tmp2);
 }
