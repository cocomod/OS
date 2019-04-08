#include <stdio.h>
#include <unistd.h>     //for pipe()
#include <string.h>     //for memset()
#include <stdlib.h>     //for exit()
int main()
{
    int fd[2];
    char buf[200]={0};
    pid_t p;
	if(-1 == pipe(fd))//创建管道
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
	p = fork();//创建进程
	if(p > 0)
	{
		close(fd[0]);//关闭父进程中的读描述符
		while(1)
		{
			printf("父进程输入:");
			scanf("%s",buf);
			write(fd[1],buf,strlen(buf));
			memset(buf,0,sizeof(buf));
		}	
	}
	else if(p == 0)
	{
		close(fd[1]);//关闭子进程中的写描述符
		while(1)
		{
			read(fd[0],buf,sizeof(buf));
			printf("子进程输出:%s\n",buf);
			sleep(1);		
		}
	}
    return 0;
}
