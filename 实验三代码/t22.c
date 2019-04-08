#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
int ticketCount = 1000;
sem_t * mutex;
void *sell(void *args){
	sem_wait(mutex);
	int n = 10,temp;
	printf("共计卖%d张票\n",n);
	for(int i = 1;i<=n;i++){
		sleep(1);
		temp=ticketCount;
		pthread_yield();
		temp=temp-1;
		pthread_yield();
		ticketCount=temp;
		printf("卖第%d张票，剩余%d张票\n",i,ticketCount);
	}
	sem_post(mutex);
	return NULL;
}
void *refund(void *args){
	sem_wait(mutex);
	int n = 5,temp;
	printf("共计退%d张票\n",n);
	for(int i = 1;i<=n;i++){
		sleep(1);
		temp=ticketCount;
		pthread_yield();
		temp=temp+1;
		pthread_yield();
		ticketCount=temp;
		printf("退第%d张票，剩余%d张票\n",i,ticketCount);
	}
	sem_post(mutex);
	return NULL;
}
int main(){
	mutex = sem_open("mutex",O_CREAT,0666,1);
	pthread_t p1,p2;
	pthread_create(&p1,NULL,sell,NULL);
	pthread_create(&p2,NULL,refund,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	printf("剩余车票票数为%d\n",ticketCount);
	sem_close(mutex);
	sem_unlink("mutex");
	return 0;
}
