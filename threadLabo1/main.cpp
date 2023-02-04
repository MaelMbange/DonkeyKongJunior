#include <iostream>
#include <threads.h>
#include <signal.h>
#include <unistd.h>

void Exercice1(void);
void Exercice2(void);
void Exercice3(void);
void Exercice4(void);

void* thread1(void* );
void* thread2(void* );

void handlerAlarm(int);

int main(int argc, char* argv[])
{
    // Exercice1();
    Exercice2();
    return 0; 
}

void * thread1(void *i){
    // std::cout << "Debut Thread= "<< pthread_self() << std::endl;
    // std::cout << "value= " << *(int*)i << std::endl;
    fprintf(stdout,"Debut Thread= %lld\nvalue= %d\n",pthread_self(),*(int*)i);
    int j = *(int*)i;
    free(i);

    struct timespec time = {1,700'000'000};
    nanosleep(&time,NULL);
    pthread_exit(NULL);
}

void* thread2(void* i){
    fprintf(stdout,"Debut Thread= %lld\n",pthread_self());

    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig,SIGALRM);
    sigprocmask(SIG_UNBLOCK,&sig,NULL);
    alarm(5);
    pause();
    fprintf(stdout,"Fin Thread= %lld\n",pthread_self());
    pthread_exit(NULL);
}

void Exercice1(void){
    pthread_t vec[4];    

    for(int x = 0; x < sizeof(vec)/sizeof(pthread_t);x++)
    {
        int* i = (int*)malloc(sizeof(int));
        *i = x;
        pthread_create(&vec[x],NULL,thread1,(void*)i);
    }

    for(pthread_t& x: vec)
    {
        pthread_join(x,NULL);
    }   
}


void Exercice2(void){
    struct sigaction sig;
    sig.sa_flags = 0;
    sig.sa_handler = handlerAlarm;
    sigemptyset(&sig.sa_mask);
    sigaction(SIGALRM,&sig,NULL);

    sigprocmask(SIG_BLOCK,&sig.sa_mask,NULL);

    pthread_create();

}
void Exercice3(void){

}
void Exercice4(void){

}


void handlerAlarm(int)
{
    fprintf(stdout,"Alarm Thread= %lld\n",pthread_self());
}