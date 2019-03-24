#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main(){
  pid_t p1,p2,p3,p4; 
  p1 = fork();
  if(p1 > 0){
    sleep(1);
    while((p3=fork())==-1);
    if(p3>0)sleep(1);
    else if(p3 == 0){
    while(1){printf("p3是p1的子进程pid = %d, p3的父进程为ppid = %d\n", getpid(), getppid());sleep(1);}
    }
    while(1){printf("p1为根进程，pid = %d\n", getpid());sleep(1);}
  }else if(p1 == 0){
    while((p2=fork())==-1);
    if(p2>0){
      sleep(1);
      while((p4=fork())==-1);
      if(p4>0)sleep(1);
      else if(p4 == 0){
      while(1){printf("p4是p2的子进程pid = %d, p4的父进程为ppid = %d\n", getpid(), getppid());sleep(1);}
      }
      exit(0);
      while(1){printf("p2是p1的子进程pid = %d, p2的父进程为ppid = %d\n", getpid(), getppid());sleep(1);}
    }else if(p2 == 0){
    while(1){printf("p5是p2的子进程pid = %d, p5的父进程为ppid = %d\n", getpid(), getppid());sleep(1);}
    }
  }
  return 0;
}

