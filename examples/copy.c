#include <stdio.h>
#include <stdlib.h>

#include <string.h>

int main()
{
char substr[5];
substr[4] = '\0';
strncpy(substr, "hi", 5);
printf("%s",substr);
}
