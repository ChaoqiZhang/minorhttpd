#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

int writen(int fd,char *message,int messagesize){
    
    int nwrite;
    int nleft;
    char *ptr;
    
    nleft = messagesize;
    ptr = message;

    while(nleft > 0){
        if((nwrite = write(fd,ptr,nleft)) < 0){
            if(errno == EINTR)
                continue;
            else
                return -1;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }

    return messagesize-nleft;
}
