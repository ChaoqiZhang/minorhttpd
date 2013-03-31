#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<syslog.h>
#include<string.h>
#include<sys/time.h>
#include<signal.h>
#include<sys/resource.h>

#ifndef MINORHTTPD_SERV_DAEMON_H
    #define MINORHTTPD_SERV_DAEMON_H
    #include "daemon.h"
#endif

#define ERR_BUFSIZE  1024
#define PID_BUFSIZE  50
char errmessage[ERR_BUFSIZE];
char pidmessage[PID_BUFSIZE];

void error_record_show_exit(){

    syslog(LOG_ERR,"%s",errmessage);
    exit(-1);

}

int lock_daemon_file(int fd){
    
    struct flock f;
    f.l_type = F_WRLCK;
    f.l_start = 0;
    f.l_whence = SEEK_SET;
    f.l_len = 0;
    return (fcntl(fd,F_SETLK,&f));

}

void make_process_daemon(){
    
    int pid;
    int err;
    int lockfd;
    int iter_fd;
    int fd0,fd1,fd2;
    struct rlimit rlim;
    struct sigaction act;

    openlog(DAEMON_LOG_IDENT,LOG_CONS,LOG_DAEMON);

    umask(0);

    lockfd = open(DAEMON_LOCK_FILE,O_RDWR|O_CREAT,DAEMON_LOCK_FILE_MODE);
    if(lockfd < 0){
        err = errno;
        snprintf(errmessage,ERR_BUFSIZE,"Error , can not create %s : %s , errno : %d",\
                DAEMON_LOCK_FILE,strerror(err),err);
        syslog(LOG_ERR,"%s",errmessage);
        printf("%s\n",errmessage);
        if(errno == 13){
            exit(-1);
        }
    }

    if((pid = fork()) < 0){
        snprintf(errmessage,ERR_BUFSIZE,"Error , call fork error : %s",strerror(errno));
        error_record_show_exit();
    }else if(pid > 0){
        exit(0);
    }
    setsid();
    
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction(SIGHUP,&act,NULL) == -1){
        snprintf(errmessage,ERR_BUFSIZE,"Error , call sigaction error : %s",strerror(errno));
        error_record_show_exit();
    }
    if((pid = fork()) < 0){
        snprintf(errmessage,ERR_BUFSIZE,"Error , call fork error : %s",strerror(errno));
        error_record_show_exit();
    }else if(pid > 0){
        exit(0);
    }

    chdir("/");
    

    if(lock_daemon_file(lockfd) < 0){
        snprintf(errmessage,ERR_BUFSIZE,"Error , can not lock %s : %s", \
                DAEMON_LOCK_FILE,strerror(errno));
        error_record_show_exit();
    }


    if(getrlimit(RLIMIT_NOFILE,&rlim) < 0){
        snprintf(errmessage,ERR_BUFSIZE,"Error , call getrlimit error : %s",strerror(errno));
        error_record_show_exit();
    }
    if(rlim.rlim_max = RLIM_INFINITY){
        rlim.rlim_max = 1024;
    }
    for(iter_fd = 0 ; iter_fd < rlim.rlim_max ; iter_fd ++){
        if(iter_fd != lockfd)
            close(iter_fd);
    }

    fd0 = open("/dev/null",O_RDWR);
    fd1 = dup(fd0);
    fd2 = dup(fd0);
    
    if(fd0 != 0 || fd1 != 1 || fd2 != 2){
        syslog(LOG_ERR,"Error,something wrong with the file descriptors :  %d %d %d",\
                fd0,fd1,fd2);
        exit(-1);
    }

    ftruncate(lockfd,0);
    sprintf(pidmessage,"pid : %d",getpid());
    write(lockfd,pidmessage,strlen(pidmessage)+1);
    syslog(LOG_ERR,"Make process daemon successfully");
    return ;
}
