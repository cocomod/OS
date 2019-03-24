#include <stdio.h>
#include <unistd.h>
int main(){
pid_t p;
p = fork();
if(p > 0){
int res = execl("/etc/alternatives/vi","vi",NULL);
if(res == -1)
	perror("execl");
}
else if(p == 0)
 while(1){}
return 0;
}
