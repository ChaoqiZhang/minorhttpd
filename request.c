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


extern const char *ROOT_PATH;

struct thread_arg{
    int requestfd;
    char requestmessage[HTTP_MESSAGE_LENGTH];
};

void wait_child_thread(int);
void *get_request_obj(void *);
extern int solve_http_request_method(char *);
extern void solve_http_request(int,int,char *,char *);

void deal_http_request(int request_fd,char *message,int messagelen){
    
    pthread_t thread;
    pthread_attr_t attr;
    struct thread_arg reqinfo;
    
    signal(SIGUSR1,wait_child_thread);

    openlog(DAEMON_LOG_IDENT,LOG_CONS,LOG_DAEMON);
    

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    
    reqinfo.requestfd = request_fd;
    strncpy(reqinfo.requestmessage,message,HTTP_MESSAGE_LENGTH);
    

    pthread_create(&thread,&attr,(void *)get_request_obj,(void *)&reqinfo);
    pause();

    return ;
}

void *get_request_obj(void *arg){
    

    char *token;
    char search[] = " \r\n";
    char param[] = "?=&";
    char request_path[HTTP_URL_LENGTH];
    char tmpmessage[HTTP_MESSAGE_LENGTH];

    struct stat request_buf;
    struct thread_arg reqinfo;

    char http_request_method[HTTP_METHOD_LENGTH];
    char http_request_url[HTTP_URL_LENGTH];
    char http_request_proto[HTTP_PROTO_LENGTH];
    
    int http_request_method_type;

    reqinfo.requestfd = (*(struct thread_arg *)arg).requestfd;
    strncpy(reqinfo.requestmessage,(*(struct thread_arg*)arg).requestmessage,\
            HTTP_MESSAGE_LENGTH);
    strncpy(tmpmessage,(*(struct thread_arg*)arg).requestmessage,HTTP_MESSAGE_LENGTH);
    kill(getpid(),SIGUSR1);
    

    /* call strtok first time */
    token = strtok(reqinfo.requestmessage,search);
    strncpy(http_request_method,token,HTTP_METHOD_LENGTH);
    
    /* call strtok second time */
    token = strtok(NULL,search);
    strncpy(http_request_url,token,HTTP_URL_LENGTH);
    
    /* call strtok third time */
    token = strtok(NULL,search);
    strncpy(http_request_proto,token,HTTP_PROTO_LENGTH);
    
    token = strtok(http_request_url,param);

    strncpy(request_path,ROOT_PATH,HTTP_URL_LENGTH);
    strncat(request_path,token,HTTP_URL_LENGTH-strlen(request_path)-1);
    
    syslog(LOG_NOTICE,"http_request_method : %s",http_request_method);
    syslog(LOG_NOTICE,"http_request_url : %s",http_request_url);
    /* Do it later */
    syslog(LOG_NOTICE,"http_request_proto : %s",http_request_proto);
    syslog(LOG_NOTICE,"http_request_path : %s",request_path);

    http_request_method_type = solve_http_request_method(http_request_method);
    solve_http_request(reqinfo.requestfd,http_request_method_type,request_path,tmpmessage);


    pthread_exit(NULL);
}

void wait_child_thread(int signo){
    //do nothing
}
