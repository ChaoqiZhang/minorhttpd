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
#include<pthread.h>
#include<dirent.h>

#ifndef MINORHTTPD_SERV_REQUEST_H
    #define MINORHTTPD_SERV_REQUEST_H
    #include "request.h"
#endif

#ifndef MINORHTTPD_SERV_HEADER_H
    #define MINORHTTPD_SERV_REQUEST_H
    #include "header.h"
#endif

#ifndef MINORHTTPD_SERV_COMMUNICATE_H
    #define MINORHTTPD_SERV_COMMUNICATE_H
    #include "communicate.h"
#endif

void response4xx(int,int);
void list_directory_content(int,char *);

extern pthread_mutex_t sockfd_mutex_arr[SERV_MAX_DESCRIPTOR];
extern int  writen(int,char *,int);

void response(int reqhandfd,char *http_request_path){
    
    DIR *dirp;
    struct stat reqstat;
    
    pthread_mutex_lock(&sockfd_mutex_arr[reqhandfd]);

    if(lstat(http_request_path,&reqstat) != 0){
        if(errno == EACCES){
            response4xx(reqhandfd,403);
        }else{
            response4xx(reqhandfd,404);
        }
    }else{
        if(S_ISREG(reqstat.st_mode)){
            //solve reg file
        }else if(S_ISDIR(reqstat.st_mode)){
            if((dirp = opendir(http_request_path)) == NULL){
                if(errno == EACCES){
                    response4xx(reqhandfd,403);
                }else{
                    response4xx(reqhandfd,404);
                }
            }else{
                list_directory_content(reqhandfd,http_request_path);
            }
        }else{
            response4xx(reqhandfd,404);
        }
    }
    
    pthread_mutex_unlock(&sockfd_mutex_arr[reqhandfd]);
    return ;
}

void list_directory_content(int reqhandfd,char *http_request_path){
    
    extern char *HEADER200;
    extern char *CONTYPEHTML;
    extern char *PAGELIST;
    extern char *DIR_PATH;
    extern char *ROOT_PATH;

    char message[HTTP_MESSAGE_LENGTH];
    char content[HTTP_MESSAGE_LENGTH];
    char conlen[HTTP_MESSAGE_LENGTH];
    char tmp[HTTP_MESSAGE_LENGTH];
    char list[HTTP_MESSAGE_LENGTH];
    char file[PATH_MAX];
    char tmppath[PATH_MAX];
    int fd,nread;
    char *ptr;

    struct stat buf;
    char pathlist[PATH_MAX];
    struct dirent **dirp;
    int nfile;
    int nfileiter;

    memset(list,0,sizeof(list));

    strncpy(message,HEADER200,HTTP_MESSAGE_LENGTH);
    strncat(message,CONTYPEHTML,HTTP_MESSAGE_LENGTH-strlen(message)-1);

    strncpy(file,DIR_PATH,PATH_MAX);
    strncat(file,PAGELIST,PATH_MAX-strlen(file)-1);

    fd = open(file,O_RDONLY);
    read(fd,content,HTTP_MESSAGE_LENGTH);
    close(fd);

    ptr = http_request_path;
    ptr = ptr + strlen(ROOT_PATH);
    
    strncpy(tmp,content,HTTP_MESSAGE_LENGTH);
    sprintf(content,tmp,ptr,ptr,"%s");
    
    nfileiter = 0;
    nfile = scandir(http_request_path,&dirp,0,alphasort);
    while(nfileiter < nfile){
        strncpy(tmppath,http_request_path,PATH_MAX);
        strncat(tmppath,dirp[nfileiter]->d_name,PATH_MAX-strlen(tmppath)-1);
        if(lstat(tmppath,&buf) == -1){
            continue;   
        }else{
            if(S_ISDIR(buf.st_mode)){
                strncat(list,"[",HTTP_MESSAGE_LENGTH-strlen(list)-1);
                strncat(list,"dir",HTTP_MESSAGE_LENGTH-strlen(list)-1);
                strncat(list,"] ",HTTP_MESSAGE_LENGTH-strlen(list)-1);
            }
            
            strncat(list,"<a href=\"",HTTP_MESSAGE_LENGTH-strlen(list)-1);
            strncat(list,ptr,HTTP_MESSAGE_LENGTH-strlen(list)-1);
            strncat(list,dirp[nfileiter]->d_name,HTTP_MESSAGE_LENGTH-strlen(list)-1);
            strncat(list,"\">",HTTP_MESSAGE_LENGTH-strlen(list)-1);
            strncat(list,dirp[nfileiter]->d_name,HTTP_MESSAGE_LENGTH-strlen(list)-1);
            strncat(list,"</a><br />",HTTP_MESSAGE_LENGTH-strlen(list)-1);
        }   
        nfileiter ++;
    }
    free(dirp);
    strncpy(tmp,content,HTTP_MESSAGE_LENGTH);
    sprintf(content,tmp,list);

    sprintf(conlen,"Content-Length: %d\r\n\r\n",(int)strlen(content));
    strncat(message,conlen,HTTP_MESSAGE_LENGTH-strlen(conlen)-1);

    writen(reqhandfd,message,strlen(message)+1);
    writen(reqhandfd,content,strlen(content)+1);
}

void response4xx(int reqhandfd,int flag){
    
    extern char *HEADER403;
    extern char *HEADER404;
    extern char *CONTYPEHTML;
    extern char *PAGE404;
    extern char *PAGE403;
    extern char *DIR_PATH;
    
    char message[HTTP_MESSAGE_LENGTH];
    char conlen[HTTP_MESSAGE_LENGTH];
    char file[HTTP_MESSAGE_LENGTH];
    int filelength;
    int nread;
    int fd;

    struct stat buf;
    char path4xx[PATH_MAX];

    switch(flag){
        case RES404:
            strncpy(message,HEADER404,HTTP_MESSAGE_LENGTH);
            strncat(message,CONTYPEHTML,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            strncpy(path4xx,DIR_PATH,PATH_MAX);
            strncat(path4xx,PAGE404,PATH_MAX-strlen(path4xx)-1);
            syslog(LOG_NOTICE,"%s",path4xx);
            break;
        case RES403:
            strncpy(message,HEADER403,HTTP_MESSAGE_LENGTH);
            strncat(message,CONTYPEHTML,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            strncpy(path4xx,DIR_PATH,PATH_MAX);
            strncat(path4xx,PAGE403,PATH_MAX-strlen(path4xx)-1);
            syslog(LOG_NOTICE,"%s",path4xx);
            break;
    }
    

    lstat(path4xx,&buf);
    filelength = buf.st_size;
    syslog(LOG_NOTICE,"%d",filelength);
    sprintf(conlen,"Content-Length: %d\r\n\r\n",filelength);
    strncat(message,conlen,HTTP_MESSAGE_LENGTH-strlen(message)-1);
    writen(reqhandfd,message,strlen(message)+1);
    fd = open(path4xx,O_RDONLY);
    while((nread = read(fd,file,HTTP_MESSAGE_LENGTH))){
        writen(reqhandfd,file,nread);
    }
    close(fd);
    
}
