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
#include<time.h>

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
void responsereg(int,char *,char *);
void get_request_file_ext(char *,char*);
void dynamic_cgi_php(int,char *);

extern pthread_mutex_t sockfd_mutex_arr[SERV_MAX_DESCRIPTOR];
extern int  writen(int,char *,int);

void response(int reqhandfd,char *http_request_path,char *http_request_message){
    

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
            responsereg(reqhandfd,http_request_path,http_request_message);               
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
    int tmplen;
    
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
        tmplen = strlen(tmppath);
        if(tmppath[tmplen-1] != '/'){
            tmppath[tmplen] = '/';
            tmppath[tmplen+1] = '\0';
        }
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
            tmplen = strlen(list);
            if(list[tmplen-1]!='/'){
                list[tmplen] = '/';
                list[tmplen+1] = '\0';
            }
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

    writen(reqhandfd,message,strlen(message));
    writen(reqhandfd,content,strlen(content));
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
            break;
        case RES403:
            strncpy(message,HEADER403,HTTP_MESSAGE_LENGTH);
            strncat(message,CONTYPEHTML,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            strncpy(path4xx,DIR_PATH,PATH_MAX);
            strncat(path4xx,PAGE403,PATH_MAX-strlen(path4xx)-1);
            break;
    }
    

    lstat(path4xx,&buf);
    filelength = buf.st_size;
    sprintf(conlen,"Content-Length: %d\r\n\r\n",filelength);
    strncat(message,conlen,HTTP_MESSAGE_LENGTH-strlen(message)-1);
    writen(reqhandfd,message,strlen(message));
    fd = open(path4xx,O_RDONLY);
    while((nread = read(fd,file,HTTP_MESSAGE_LENGTH))){
        if(nread < 0){
            if(errno == EINTR)
                continue;
            else{
                break;
            }
        }
        writen(reqhandfd,file,nread);
    }
    close(fd);
    
}

void responsereg(int reqhandfd,char *http_request_path,char *http_request_message){
    
    FILE *fp;
    int typeflag;

    int fd;
    int nread;
    char *ptr;
    struct stat buf;
    char ext[PATH_MAX];
    char message[HTTP_MESSAGE_LENGTH];
    char conlen[HTTP_MESSAGE_LENGTH];
    char content[HTTP_MESSAGE_LENGTH];

    extern char *ROOT_PATH;
    extern char *HEADER200;
    extern char *CONTYPEHTML;
    extern char *CONTYPEPLAIN;
    extern char *CONTYPECSS;
    extern char *CONTYPEGIF;
    extern char *CONTYPEPNG;
    extern char *CONTYPEJPEG;
    extern char *CONTYPEJS; 
    extern char *SERVER;
    extern char *ACCRANGES;
    
    time_t timep;
    int templen;


    ptr = http_request_path + strlen(ROOT_PATH);
    get_request_file_ext(ptr,ext);

    if(lstat(http_request_path,&buf) == -1){
        if(errno == EACCES){
            response4xx(reqhandfd,403);
        }else{
            response4xx(reqhandfd,404);
        }
    }else{
    
        sprintf(conlen,"Content-Length: %d\r\n",(int)(buf.st_size));
        typeflag = 0;

        if((fp = fopen(http_request_path,"r")) == NULL){
            if(errno == EACCES){
                response4xx(reqhandfd,403);
            }else{
                response4xx(reqhandfd,404);
            }
        }else{

            strncpy(message,HEADER200,HTTP_MESSAGE_LENGTH);
            if(strcmp(ext,"html") == 0 || strcmp(ext,"htm") == 0){
                strncat(message,CONTYPEHTML,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            }else if(strcmp(ext,"php") == 0){
                dynamic_cgi_php(reqhandfd,http_request_message);
                return ;
            }else if(strcmp(ext,"text") == 0 || strcmp(ext,"c") == 0 || strcmp(ext,"c++")==0 \
                    || strcmp(ext,"pl")==0 || strcmp(ext,"cc") == 0 || strcmp(ext,"h") == 0){
                strncat(message,CONTYPEPLAIN,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            }else if(strcmp(ext,"css") == 0){
                strncat(message,CONTYPECSS,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            }else if(strcmp(ext,"gif") == 0){
                typeflag = 1;
                strncat(message,CONTYPEGIF,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            }else if(strcmp(ext,"png") == 0){
                typeflag = 1;
                strncat(message,CONTYPEPNG,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            }else if(strcmp(ext,"jpeg") == 0 || strcmp(ext,"jpg") == 0 || strcmp(ext,"jpe") == 0){
                typeflag = 1;
                strncat(message,CONTYPEJPEG,HTTP_MESSAGE_LENGTH-strlen(message)-1);
            }
        }

        strncat(message,conlen,HTTP_MESSAGE_LENGTH-strlen(message)-1);
        strncat(message,SERVER,HTTP_MESSAGE_LENGTH-strlen(message)-1);
        strncat(message,ACCRANGES,HTTP_MESSAGE_LENGTH-strlen(message)-1);
        time(&timep);
        sprintf(conlen,"Date: %s",asctime(gmtime(&timep)));
        strncat(message,conlen,HTTP_MESSAGE_LENGTH-strlen(message)-1);
        
        int templen = strlen(message);
        message[templen-1]='\0';

        strncat(message,"\r\n\r\n",HTTP_MESSAGE_LENGTH-strlen(message)-1);
    
        writen(reqhandfd,message,strlen(message));
        
        if(typeflag == 1){
            fclose(fp);
            if((fp = fopen(http_request_path,"rb")) == NULL){
                if(errno == EACCES){
                    response4xx(reqhandfd,403);
                }else{
                    response4xx(reqhandfd,404);
                }
            }
        }
        
        while((nread = fread(content,sizeof(char),HTTP_MESSAGE_LENGTH,fp)) ){
            if(nread < 0){
                if(errno == EINTR){
                    continue;
                }
                else{
                    break;
                }

            }else{
                writen(reqhandfd,content,nread);
            }
        }
        fclose(fp);
    }
    return ;
}

void get_request_file_ext(char *ptr,char *ext){
    
    int tmplen = strlen(ptr);
    char *p ;

    for(p=ptr+tmplen;p>=ptr;p--){
        if(*p == '.'){
            strncpy(ext,p+1,PATH_MAX);
        }
    }
    
    return ;
}
