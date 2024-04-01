#include<stdio.h>
#include<unistd.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define DATA_SIZE 100

int main(int argc,char *argv[]){
    char filename[DATA_SIZE] ;
    snprintf(filename,sizeof(filename),"/tmp/%s",argv[1]);
    
    int fd = open(filename, O_WRONLY);//| O_CREAT | O_TRUNC,
    if(fd < 0)
        return errno;
    
    char data[DATA_SIZE];
    memset(data, 'a', DATA_SIZE); 

    int bytesWritten = write(fd, data, DATA_SIZE); 
    if(bytesWritten == DATA_SIZE)
    {
        close(fd);
        return 0;
    }
        close(fd);
    return errno;
}