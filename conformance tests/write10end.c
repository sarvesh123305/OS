#include<stdio.h>
#include<unistd.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc,char *argv[]){
    if(argc < 2)
        return errno;
    char data[] ="1234567890";

    int fd = open(argv[1],O_WRONLY | O_APPEND);
    if(fd < 0)
        return errno;

    if(lseek(fd,0,SEEK_END) == -1) // move to eof
    {
        close(fd);
        return 1;
    }

    if(write(fd,data,sizeof(data) ) == -1)
    {
        close(fd);
        return errno;
    }
    close(fd);
    return 0;
}