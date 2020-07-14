#include <stdio.h>
#include <stdlib.h>
void some()
{
    printf("called func");
    exit(1);
}
int main()
{
  some();
  printf("doesn't reach");
}

