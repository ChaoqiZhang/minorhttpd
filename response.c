#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<syslog.h>
#include<limits.h>
#include<errno.h>

#ifndef MINORHTTPD_SERV_REQUEST_H
    #define MINORHTTPD_SERV_REQUEST_H
    #include "request.h"
#endif

#ifndef MINORHTTPD_SERV_HEADER_H
    #define MINORHTTPD_SERV_REQUEST_H
    #include "header.h"
#endif



void response404(int);

void response(int reqhandfd,int resflag,char *http_request_path){
    
    switch(resflag){
        case RES200:
            break;
        case RES500:
            break;
        case RES404:
            response404(reqhandfd);
            break;
    }   
    
    return ;
}

void response404(int reqhandfd){
 
    extern char *HEADER404;
    extern char *CONTYPEHTML;
    extern char *PAGE404;
    extern char *DIR_PATH;
    
    struct stat buf;
    char path404[PATH_MAX];
    char message[HTTP_MESSAGE_LENGTH];
    char conlen[HTTP_MESSAGE_LENGTH];
    char file[HTTP_MESSAGE_LENGTH];
    int filelength;
    int nread;
    int fd;
    

    strncpy(message,HEADER404,HTTP_MESSAGE_LENGTH);
    strncat(message,CONTYPEHTML,HTTP_MESSAGE_LENGTH-strlen(message)-1);
    
    strncpy(path404,DIR_PATH,PATH_MAX);
    strncat(path404,PAGE404,PATH_MAX-strlen(path404)-1);
    
    syslog(LOG_NOTICE,"%s",path404);

    lstat(path404,&buf);
    filelength = buf.st_size;
    syslog(LOG_NOTICE,"%d",filelength);
    sprintf(conlen,"Content-Length: %d\r\n\r\n",filelength);
    strncat(message,conlen,HTTP_MESSAGE_LENGTH-strlen(message)-1);
    write(reqhandfd,message,strlen(message)+1);
    fd = open(path404,O_RDONLY);
    while((nread = read(fd,file,HTTP_MESSAGE_LENGTH))){
        write(reqhandfd,file,nread);
    }
    close(fd);
    
}
