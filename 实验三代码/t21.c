#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
int ticketCount = 1000;
void *sell(void *args){
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
	return NULL;
}
void *refund(void *args){
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
	return NULL;
}
int main(){
	pthread_t p1,p2;
	pthread_create(&p1,NULL,sell,NULL);
	pthread_create(&p2,NULL,refund,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	printf("剩余车票票数为%d\n",ticketCount);
	return 0;
}
