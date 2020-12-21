#include <stdio.h>
#include <stdlib.h>

int main(){
    puts(getenv("PWD"));
}
//char *getenv(const char *name);