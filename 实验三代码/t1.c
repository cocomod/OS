#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <fcntl.h>
int main()
{
	sem_t *sem1, *sem2, *sem3;
	pid_t p1,p2,p3;
	sem1 = sem_open("sem1",O_CREAT,0666,0);
	sem2 = sem_open("sem2",O_CREAT,0666,0);
	sem3 = sem_open("sem3",O_CREAT,0666,0);
	p1 = fork(); //创建进程P1
	if(p1 == 0)
	{
		printf("我是P1\n");
		sem_post(sem1);//p1
	}
	else if(p1>0)
	{
		p2 = fork();
		if(p2 == 0){
		sem_wait(sem1);
		printf("我是P2\n");
		sem_post(sem1);
		sem_post(sem2);
		}else if(p2>0){
			//sleep(1);
			p3 = fork();
			if(p3 == 0){
			sem_wait(sem1);
			printf("我是P3\n");
			sem_post(sem1);
			sem_post(sem3);
			}else if(p3>0){
			sem_wait(sem2);
			sem_wait(sem3);
			printf("我是p4\n");
			sem_post(sem2);
			sem_post(sem3);}}
	}
	sem_close(sem1);
	sem_close(sem2);
	sem_close(sem3);
	sem_unlink("sem1");
	sem_unlink("sem2");
	sem_unlink("sem3");
	return 0;
}
