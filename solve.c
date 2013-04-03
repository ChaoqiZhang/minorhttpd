#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<limits.h>
#include<errno.h>
#include<syslog.h>
#include<sys/types.h>
#include<sys/stat.h>

#ifndef MINORHTTPD_SERV_REQUEST_H
    #define MINORHTTPD_SERV_REQUEST_H
    #include "request.h"
#endif

#ifndef MINORHTTPD_SERV_SOLVE_H
    #define MINORHTTPD_SERV_SOLVE_H
    #include "solve.h"
#endif


void solve_get(int,char *);
extern void response(int,int,char *);

int solve_http_request_method(char *http_request_method){
    
    int flag_method = FLAG_ERR;
    if(strncmp(http_request_method,"GET",HTTP_METHOD_LENGTH) == 0){
        flag_method = FLAG_GET;
    }
    return flag_method;

}

void solve_http_request(int requestfd,int flag_method,char *http_request_path){
    
    switch(flag_method){
        case FLAG_GET:
            solve_get(requestfd,http_request_path);
            break;
    }
    return ;
}

void solve_get(int requestfd,char *http_request_path){
    
    struct stat reqstat;
    if(lstat(http_request_path,&reqstat) != 0){
        if(errno == ENOENT){
            response(requestfd,404,NULL);
        }
    }

}

