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
void* thread3_1(void* );
void* thread3_2(void* );
void* thread3_3(void* );

void handlerAlarm(int);
void handlerUSR1(int);
void handlerUSR2(int);

int main(int argc, char* argv[])
{
    // Exercice1();
    // Exercice2();
    Exercice3();

    return 0; 
}
void Bip(void){
    fprintf(stdout,"\a");
    fflush(stdout);
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
    // fprintf(stdout,"Debut pause\n");
    pause();
    // fprintf(stdout,"Fin pause\n");

    fprintf(stdout,"Fin Thread= %lld\n",pthread_self());
    pthread_exit(NULL);
}
void* thread3_1(void* ){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGALRM);
    sigprocmask(SIG_UNBLOCK,&mask,NULL);

    fprintf(stdout,"\033[91mDebut Thread1= %lld\033[0m\n",pthread_self());
    pause();
    fprintf(stdout,"\033[91mFin Thread1= %lld\033[0m\n",pthread_self());
    pthread_exit(NULL);
}
void* thread3_2(void* ){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGUSR1);
    sigprocmask(SIG_UNBLOCK,&mask,NULL);

    fprintf(stdout,"\033[92mDebut Thread2= %lld\033[0m\n",pthread_self());
    pause();
    fprintf(stdout,"\033[92mFin Thread2= %lld\033[0m\n",pthread_self());
    pthread_exit(NULL);
}
void* thread3_3(void* ){
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGUSR2);
    sigprocmask(SIG_UNBLOCK,&mask,NULL);

    fprintf(stdout,"\033[96mDebut Thread3= %lld\033[0m\n",pthread_self());
    alarm(5);
    pause();
    fprintf(stdout,"\033[96mFin Thread3= %lld\033[0m\n",pthread_self());
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

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGALRM);
    sigprocmask(SIG_BLOCK,&mask,NULL);

    pthread_t thread;
    pthread_create(&thread,NULL,thread2,NULL);

    pthread_join(thread,NULL);

}
void Exercice3(void){

    pthread_t threads[3];

    void*(*tab[3])(void*) {thread3_1,thread3_2,thread3_3};
    
    struct sigaction signal;
    signal.sa_flags = 0;
    signal.sa_handler = handlerAlarm;
    sigemptyset(&signal.sa_mask);
    sigaction(SIGALRM,&signal,NULL);

    signal.sa_flags = 0;
    signal.sa_handler = handlerUSR1;
    sigemptyset(&signal.sa_mask);
    sigaction(SIGUSR1,&signal,NULL);

    signal.sa_flags = 0;
    signal.sa_handler = handlerUSR2;
    sigemptyset(&signal.sa_mask);
    sigaction(SIGUSR2,&signal,NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask,SIGINT);
    sigprocmask(SIG_BLOCK,&mask,NULL);

    for (size_t i = 0; i < 3; i++)
    {
        /*if(i == 0)
            pthread_create(&threads[0],NULL,thread3_1,NULL);
        else if(i == 1)
            pthread_create(&threads[1],NULL,thread3_2,NULL);
        else if(i == 2)
            pthread_create(&threads[2],NULL,thread3_3,NULL);*/
        pthread_create(&threads[i],NULL,tab[i],NULL);
    }
    struct timespec time{1,0};
    nanosleep(&time,NULL);
    fprintf(stdout,"\033[0m==============================\n");
    nanosleep(&time,NULL);

    pthread_kill(threads[1],SIGUSR1);
    pthread_kill(threads[2],SIGUSR2);

    for(pthread_t& x: threads)
    {
        pthread_join(x,NULL);
    } 

    /*time = {0,500'000'000};
    for (size_t i = 0; i < 10; i++)
    {
        Bip();
        nanosleep(&time,NULL);
    }*/
    
    
}
void Exercice4(void){

}


void handlerAlarm(int){
    fprintf(stdout,"\033[5m\aAlarm\033[0m Thread= %lld\n",pthread_self()); 
}

void handlerUSR1(int){
    fprintf(stdout,"User1 Thread= %lld\n",pthread_self());
}
void handlerUSR2(int){
    fprintf(stdout,"User2 Thread= %lld\n",pthread_self());
}