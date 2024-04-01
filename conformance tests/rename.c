#include<stdio.h>
#include<unistd.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define DATA_SIZE 100

int main(int argc,char *argv[]){
   
    if (!(access(argv[1], F_OK) != -1)) {
        return errno;
    }
    int value = rename(argv[1],argv[2]);
    if(!value)
        return 0;

    return errno;
}