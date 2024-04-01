#include<stdio.h>
#include<unistd.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define DATA_SIZE 100

int main(int argc,char *argv[]){
   
    int fd = open(argv[1], O_RDWR);//| O_CREAT | O_TRUNC,
    if(fd < 0)
        return errno;
    
    int offset = lseek(fd,5,SEEK_SET);
    if(offset == -1)
    {
        close(fd);
        return errno;
    }
    
    char buffer[6];
    int bytesRead = read(fd,buffer,6);
    if(bytesRead == 6)
        return 0;
    
    close(fd);
    return errno;
}