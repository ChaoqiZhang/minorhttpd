#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<mysql/mysql.h>
#include<syslog.h>
#include<strings.h>
#include<string.h>
#include<errno.h>
#include<sys/select.h>

#ifndef MINORHTTPD_SERV_DAEMON_H
    #define MINORHTTPD_SERV_DAEMON_H
    #include "daemon.h"
#endif

#ifndef MINORHTTPD_SERV_COMMUNICATE_H
    #define MINORHTTPD_SERV_COMMUNICATE_H
    #include "communicate.h"
#endif


int clientfd[SERV_MAX_DESCRIPTOR];
int max_clientfd = -1;
int cur_clientfd_ready;

fd_set readfds;

void init_clientfd_array();
extern void deal_http_request(int,char *,int);

void communicate(){
    
    int iterfd;
    int nread;
    int tmpfd;
    int listenfd;
    int sockopt;
    int client_addr_len;
    int clientfd_array_iter;
    struct sockaddr_in server,client;

    fd_set tmpreadfds;
    
    char buf_recv[SERV_RECV_BUF_LEN];
    char buf_IPV4[SERV_IPV4_BUF_LENGTH];

    openlog(DAEMON_LOG_IDENT,LOG_CONS,LOG_DAEMON);

    if((listenfd=socket(AF_INET,SOCK_STREAM,0)) == -1){
        syslog(LOG_ERR,"Error , fail to create listen socket file descriptor :%s",\
                strerror(errno));
        exit(-1);
    }
    

    bzero(&server,sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(SERV_LISTEN_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    sockopt = SERV_KEEPALIVE_OPTION;

    if(setsockopt(listenfd,SOL_SOCKET,SO_KEEPALIVE,(void *)&sockopt,\
                sizeof(sockopt)) == -1){
        syslog(LOG_ERR,"Error , fail to set the keep alive option of the \
                listen socket file descriptor : %s",strerror(errno));
    }

    sockopt = SERV_REUSEADDR_OPTION;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void *)&sockopt,\
                sizeof(sockopt)) == -1){
        syslog(LOG_ERR,"Error , fail to set the reuseaddr option of the \
                listen socket file descriptor : %s",strerror(errno));
    }
    
    if(bind(listenfd,(struct sockaddr *)&server,sizeof(server)) == -1){
        syslog(LOG_ERR,"Error , fail to bind the serveraddr : %s",strerror(errno));
        exit(-1);
    }
    
    if(listen(listenfd,SOMAXCONN) == -1){
        syslog(LOG_ERR,"Error , fail to listen the socket file descriptor : %s",\
                strerror(errno));
        exit(-1);
    }
    
    init_clientfd_array();
    
    FD_SET(listenfd,&readfds);
    max_clientfd = (max_clientfd > listenfd) ? max_clientfd : listenfd;

    while(1){

        tmpreadfds = readfds;
        cur_clientfd_ready = select(max_clientfd+1,&tmpreadfds,NULL,NULL,NULL);

        if(cur_clientfd_ready == -1){
            syslog(LOG_ERR,"Error , something wrong with select: %s",\
                    strerror(errno));
            continue;
        }

        if(FD_ISSET(listenfd,&tmpreadfds)){

            bzero(&client,sizeof(client));
            client_addr_len = sizeof(client);
            if((tmpfd = accept(listenfd,(struct sockaddr *)&client, \
                            &client_addr_len)) == -1){
                syslog(LOG_ERR,"Error , call accept error : %s",strerror(errno));
            }else{

            
                if(inet_ntop(AF_INET,(void *)&(client.sin_addr),buf_IPV4,\
                        SERV_IPV4_BUF_LENGTH) == NULL){
             
                    syslog(LOG_ERR,"Error , get the ipaddress error : %s",\
                            strerror(errno));
                    close(tmpfd);

                }else{

                    syslog(LOG_NOTICE,"Connection from : %s:%d",buf_IPV4,\
                        (int)htons(client.sin_port));
                }

                for(iterfd = 0 ; iterfd < SERV_MAX_DESCRIPTOR ; iterfd ++){
                
                    if(clientfd[iterfd] == -1){
                        clientfd[iterfd] = tmpfd;
                        break;
                    }

                }
            
                if(iterfd == SERV_MAX_DESCRIPTOR){
                    // do nothing
                    syslog(LOG_ERR,"Error , too many connections");
                    close(tmpfd);
                }else{
                    FD_SET(tmpfd ,&readfds);
                    max_clientfd = (max_clientfd > tmpfd)?max_clientfd:tmpfd;
                }
            }

            cur_clientfd_ready --;

            if(cur_clientfd_ready == 0)
                continue;
        }

        for(clientfd_array_iter = 0 ; clientfd_array_iter < \
                SERV_MAX_DESCRIPTOR ; clientfd_array_iter ++){
            
            if(clientfd[clientfd_array_iter] == -1)
                continue;

            if(FD_ISSET(clientfd[clientfd_array_iter],&tmpreadfds)){
                if((nread = recv(clientfd[clientfd_array_iter],buf_recv,\
                                SERV_RECV_BUF_LEN,0)) < 0){
                    if(errno == ECONNRESET){
                        syslog(LOG_ERR,"Connect reset by client");
                        FD_CLR(clientfd[clientfd_array_iter],&readfds);
                        close(clientfd[clientfd_array_iter]);
                        clientfd[clientfd_array_iter] = -1;
                        /* Connection reset by client */
                    }
                }else if(nread == 0){
                    syslog(LOG_NOTICE,"Connect close by client");
                    FD_CLR(clientfd[clientfd_array_iter],&readfds);
                    close(clientfd[clientfd_array_iter]);
                    clientfd[clientfd_array_iter] = -1;
                    /* Connection close by client */
                }else{
                    syslog(LOG_NOTICE,"Handle request from %d", \
                            clientfd[clientfd_array_iter]);
                    deal_http_request(clientfd[clientfd_array_iter],buf_recv,nread);
                }
                cur_clientfd_ready --;
                if(cur_clientfd_ready == 0)
                    break;
            }
        }

    }
    return ;   
}


void init_clientfd_array(){
    
    int cur_fd_iter = 0;

    for(cur_fd_iter = 0 ; cur_fd_iter < SERV_MAX_DESCRIPTOR ; \
            cur_fd_iter ++){
        clientfd[cur_fd_iter] = -1;
    }

    FD_ZERO(&readfds);

    return ;
}

