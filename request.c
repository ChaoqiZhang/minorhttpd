#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<limits.h>
#include<fcntl.h>
#include<syslog.h>
#include<pthread.h>
#include<sys/stat.h>
#include<signal.h>
#include<errno.h>

#ifndef MINORHTTPD_SERV_DAEMON_H
    #define MINORHTTPD_SERV_DAEMON_H
    #include "daemon.h"
#endif

#ifndef MINORHTTPD_SERV_REQUEST_H
    #define MINORHTTPD_SERV_REQUEST_H
    #include "request.h"
#endif

#ifndef MINORHTTPD_SERV_CONFIG_H
    #define MINORHTTPD_SERV_CONFIG_H
    #include "config.h"
#endif



struct thread_arg{
    int requestfd;
    char requesturl[HTTP_HEADER_FIRST_LINE];
};

void wait_child_thread(int);
void *get_request_obj(void *);

void deal_http_request(int request_fd,char *message,int messagelen){
    
    char *token;
    char search[] = " ";
    pthread_t thread;
    pthread_attr_t attr;
    struct thread_arg reqinfo;
    char request_path[HTTP_HEADER_FIRST_LINE];
    
    signal(SIGUSR1,wait_child_thread);

    openlog(DAEMON_LOG_IDENT,LOG_CONS,LOG_DAEMON);
    
    /* call strtok first time */
    token = strtok(message,search);
    
    /* call strtok second time */
    token = strtok(NULL,search);

    strcpy(request_path,ROOT_PATH);
    strcat(request_path,token);

    syslog(LOG_NOTICE,"Request URL : %s",request_path);
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    
    reqinfo.requestfd = request_fd;
    strcpy(reqinfo.requesturl,request_path);

    pthread_create(&thread,&attr,(void *)get_request_obj,(void *)&reqinfo);
    pause();

    return ;
}

void *get_request_obj(void *arg){
    
    struct stat request_buf;
    struct thread_arg reqinfo;
    
    reqinfo.requestfd = (*(struct thread_arg *)arg).requestfd;
    strcpy(reqinfo.requesturl,(*(struct thread_arg *)arg).requesturl);

    kill(getpid(),SIGUSR1);

    if(stat(reqinfo.requesturl,&request_buf) == -1){
        if(errno == ENOENT){
            char messagecontent[]="<html><body>This Is My Web Server</body></html>";
            char messageheader[4096];
            sprintf(messageheader,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n",(int)(strlen(messagecontent)+1));
            write(reqinfo.requestfd,messageheader,strlen(messageheader)+1);   
            write(reqinfo.requestfd,messagecontent,strlen(messagecontent)+1);
            syslog(LOG_NOTICE,"Send to %d",reqinfo.requestfd);
        }
    }

    pthread_exit(NULL);
}

void wait_child_thread(int signo){
    //do nothing
}
