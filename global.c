#include<pthread.h>

#ifndef MINORHTTPD_SERV_COMMUNICATE_H
    #define MINORHTTPD_SERV_COMMUNICATE_H
    #include "communicate.h"
#endif

pthread_mutex_t sockfd_mutex_arr[SERV_MAX_DESCRIPTOR];

const char *HEADER403 = "HTTP/1.1 403 Forbidden\r\n";
const char *HEADER404 = "HTTP/1.1 404 Not Found\r\n";
const char *HEADER200 = "HTTP/1.1 200 OK\r\n";
const char *HEADER501 = "HTTP/1.1 501 Not Implemented\r\n";


const char *CONTYPEHTML = "Content-Type: text/html;charset=utf-8\r\n";
const char *CONTYPEPLAIN = "Content-Type: text/plain;charset=utf-8\r\n";
const char *CONTYPECSS = "Content-Type: text/css;charset=utf-8\r\n";
const char *CONTYPEGIF = "Content-Type: image/gif\r\n";
const char *CONTYPEPNG = "Content-Type: image/x-png\r\n";
const char *CONTYPEJPEG = "Content-Type: image/jpeg\r\n";
const char *CONTYPEJS = "Content-Type: text/javascript;charset=utf-8\r\n";

const char *CONNCLOSE = "Connection: close\r\n";
const char *CONNALIVE = "Connection: keep-alive\r\n";

const char *SERVER = "Server: minorhttpd-0.9\r\n";
const char *ACCRANGES = "Accept-Ranges: bytes\r\n";


const char *PAGE404 = "/common/404.html";
const char *PAGE403 = "/common/403.html";
const char *PAGELIST = "/common/list.html";
