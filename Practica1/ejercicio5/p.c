#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void my_free(void* ptr) {
free(ptr);
ptr=NULL;
}

int main (void) {
char* p=malloc(20);
strcpy(p,"hello");
printf("%s\n",p);
my_free(p);
return 0;
}