#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

extern void make_process_daemon();
extern void communicate();

int main(int argc,char *argv[]){   
    
    make_process_daemon();
    communicate();

    return 0;
}
