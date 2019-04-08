#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
char buf[10];
sem_t *empty,* full;
void *t_input(void * args){
	int i = 0;
	while(1){
	sem_wait(empty);
	scanf("%c",&buf[i]);
	i = (i+1)%10;
	sem_post(full);
	}
	return NULL;
}
void *t_print(void * args){
	int i = 0;
	while(1){
	sem_wait(full);
	printf("输出%c\n",buf[i]);
	i = (i+1)%10;
	sem_post(empty);
	sleep(1);
	}
	return NULL;
}
int main(){
	empty = sem_open("empty",O_CREAT,0666,10);
	full = sem_open("full",O_CREAT,0666,0);
	pthread_t p1,p2;
	pthread_create(&p1,NULL,t_input,NULL);
	pthread_create(&p2,NULL,t_print,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	sem_close(empty);
	sem_close(full);
	sem_unlink("empty");
	sem_unlink("full");
	return 0;
}
