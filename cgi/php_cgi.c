#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<syslog.h>
#include<limits.h>
#include<sys/stat.h>
#include<time.h>

#ifndef MINORHTTPD_SERV_REQUEST_H
    #define MINORHTTPD_SERV_REQUEST_H
    #include "../request.h"
#endif

int find_sub_string(char *,char *,int);

void dynamic_cgi_php(int reqhandfd,char *http_request_message){
    
    pid_t pid;
    int pipefd[2];
    
    time_t timep;

    extern char *ROOT_PATH;
    extern char *HEADER200;
    extern char *CONTYPEHTML;

    char cookie[HTTP_MESSAGE_LENGTH];
    char method[HTTP_METHOD_LENGTH];
    char url[HTTP_URL_LENGTH];
    char query[HTTP_URL_LENGTH];
    char path[PATH_MAX];
    char search[]=" ";
    char seq[]="?";
    char *token;

    char message[HTTP_MESSAGE_LENGTH];
    char header[HTTP_MESSAGE_LENGTH];
    char content[HTTP_MESSAGE_LENGTH];

    int loc;
    int tmploc;
    int tmplen;

    char tmpmessage[HTTP_MESSAGE_LENGTH];
    strncpy(tmpmessage,http_request_message,HTTP_MESSAGE_LENGTH);

    token = strtok(tmpmessage,search);
    strncpy(method,token,HTTP_METHOD_LENGTH);
    token = strtok(NULL,search);
    strncpy(url,token,HTTP_URL_LENGTH);
    
    loc = strlen(url);
    url[loc] = '?';
    url[loc+1] = '\0';
    
    strncpy(path,ROOT_PATH,PATH_MAX);
    token = strtok(url,seq);
    strncat(path,token,PATH_MAX-strlen(path)-1);

    token = strtok(NULL,seq);
    if(token == NULL){
        query[0] = '\0';
    }else{
        strncpy(query,token,HTTP_URL_LENGTH);
    }

    pipe(pipefd);

    syslog(LOG_NOTICE,"%s",method);
    syslog(LOG_NOTICE,"%s",path);
    syslog(LOG_NOTICE,"%s",query);

    
    if((pid = fork()) == 0){
    
        close(pipefd[0]);
        char shellcontent[SHELL_LENGTH];
        strncpy(shellcontent,"export REDIRECT_STATUS=true;export GATEWAY_INTERFACE=CGI/1.1;",\
                SHELL_LENGTH);
        strncat(shellcontent,"export SCRIPT_FILENAME=",SHELL_LENGTH-strlen(shellcontent)-1);
        strncat(shellcontent,path,SHELL_LENGTH-strlen(shellcontent)-1);
        strncat(shellcontent,";export QUERY_STRING=",SHELL_LENGTH-strlen(shellcontent)-1);
        strncat(shellcontent,query,SHELL_LENGTH-strlen(shellcontent)-1);
        strncat(shellcontent,";php-cgi",SHELL_LENGTH-strlen(shellcontent)-1);

        
        if(dup2(pipefd[1],fileno(stdout)) == -1){
            syslog(LOG_ERR,"Call dup2 error : %s",strerror(errno));
        }

        system(shellcontent);
        fflush(NULL);
        exit(0);
    }else{
        close(pipefd[1]);
        waitpid(pid,NULL,0);
        read(pipefd[0],message,HTTP_MESSAGE_LENGTH);
        syslog(LOG_NOTICE,"%s",message);
        read(pipefd[0],message,HTTP_MESSAGE_LENGTH);
        syslog(LOG_NOTICE,"%s",message);
        
        strncpy(header,HEADER200,HTTP_MESSAGE_LENGTH);
        strncat(header,CONTYPEHTML,HTTP_MESSAGE_LENGTH-strlen(header)-1);
        
        tmploc = find_sub_string(message,"Set-Cookie",0);
        syslog(LOG_NOTICE,"%d",tmploc);
        if(tmploc != -1){
            loc = find_sub_string(message,"\r\n",tmploc);
            message[loc] = '\0';
            strncat(header,&message[tmploc],HTTP_MESSAGE_LENGTH-strlen(header)-1);
            strncat(header,"\r\n",HTTP_MESSAGE_LENGTH-strlen(header)-1);
            message[loc] = '\r';
        }
        
        time(&timep);
        sprintf(content,"Date: %s",asctime(gmtime(&timep)));
        strncat(header,content,HTTP_MESSAGE_LENGTH-strlen(header)-1);

        tmplen= strlen(header);
        if(header[tmplen-1]=='\n')
            header[tmplen-1]='\0';

        strncat(header,"\r\n",HTTP_MESSAGE_LENGTH-strlen(header)-1);

        tmploc = find_sub_string(message,"\r\n\r\n",0);
        if(tmploc != -1){
            tmploc += 4;
        }
        strncpy(content,&message[tmploc],HTTP_MESSAGE_LENGTH);
        sprintf(message,"Content-Length: %d\r\n\r\n",(int)strlen(content));
        strncat(header,message,HTTP_MESSAGE_LENGTH-strlen(header)-1);

        //syslog(LOG_NOTICE,"%s",header);
        //syslog(LOG_NOTICE,"%s",content);

        write(reqhandfd,header,strlen(header));
        write(reqhandfd,content,(int)strlen(content));

    }
    
    return ;
}

int find_sub_string(char *str,char *pattern,int loc){
    
    int lenstr = strlen(&str[loc]);
    int lenpat = strlen(pattern);

    int i = loc;
    int j = 0;

    if(lenstr < lenpat){
        return -1;
    }else{
        while(i<=lenstr-lenpat){
            j = 0;
            while(str[i+j] == pattern[j]){
                if(j == lenpat-1){
                    return i;
                }
                j++;
            }
            i++;
        }
    }
    return -1;

}

