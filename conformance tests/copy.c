#include<stdio.h>
#include<unistd.h>
#include <errno.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 4096 //Some constant to be assumed

int main(int argc,char *argv[]) {
    int sourceFile, destinationFile;
    int bytesRead, bytesWritten;
    char buffer[BUFFER_SIZE];

    sourceFile = open(argv[1], O_RDONLY);
    if (sourceFile == -1) 
        return errno;

    destinationFile = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (destinationFile == -1) {
        close(sourceFile);
        return errno;
    }

    while ((bytesRead = read(sourceFile, buffer, BUFFER_SIZE)) > 0) {
        bytesWritten = write(destinationFile, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            close(sourceFile);
            close(destinationFile);
            return 1;
        }
    }

    if (bytesRead == -1) {
        close(sourceFile);
        close(destinationFile);
        return 1;
    }

    close(sourceFile);
    close(destinationFile);

    return 0;
}
