#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>

void* FctThread(void *);

int main()
{
	pthread_t hThread;
	
	printf("thread principal : debut\n");
	
	int *p = (int *)malloc(sizeof(int));
	
	*p = 25;
	
	pthread_create(&hThread, NULL, FctThread, p);
	
	pthread_join(hThread, NULL);
	
	printf("thread principal : fin\n");

	exit(0);	
}


void* FctThread(void *param)
{
    	struct timespec temps = { 1, 500000000 };
    
	printf("thread secondaire : debut\n");
	
	printf("valeur du parametre = %d\n", *(int *)param);
	
	free(param);
	
	nanosleep(&temps, NULL);
	
	printf("thread secondaire : fin\n");
	
	pthread_exit(0);
}