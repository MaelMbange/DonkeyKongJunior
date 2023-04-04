#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define VIDE        		0
#define DKJR       		1
#define CROCO       		2
#define CORBEAU     		3
#define CLE 			4

#define AUCUN_EVENEMENT    	0

#define LIBRE_BAS		1
#define LIANE_BAS		2
#define DOUBLE_LIANE_BAS	3
#define LIBRE_HAUT		4
#define LIANE_HAUT		5

void* FctThreadEvenements(void *);
void* FctThreadCle(void *);
void* FctThreadDK(void *);
void* FctThreadDKJr(void *);
void* FctThreadScore(void *);
void* FctThreadEnnemis(void *);
void* FctThreadCorbeau(void *);
void* FctThreadCroco(void *);

void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();
void printGrille();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void DestructeurVS(void *p);

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = true;
int  delaiEnnemis = 4000;
int  positionDKJr = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;
int nbrVie = 0;

typedef struct
{
  int type;
  pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
  bool haut;
  int position;
} S_CROCO;

// --------------------------Methodes de threads-----------------------------
void* ThreadCle(void*);
void* ThreadEvent(void*);
void* ThreadDKJr(void*);
void* ThreadDK(void*);
void* ThreadScore(void*);
void* ThreadEnnemis(void*);
void* ThreadCorbeau(void*);
void* ThreadCroco(void*);

// ------------------------------------------------------------------------

	/*afficherCage(2);
	afficherCage(3);
	afficherCage(4);

	afficherRireDK();

	afficherCle(3);

	afficherCroco(11, 2);
	afficherCroco(17, 1);
	afficherCroco(0, 3);
	afficherCroco(12, 5);
	afficherCroco(18, 4);

	afficherDKJr(11, 9, 1);
	afficherDKJr(6, 19, 7);
	afficherDKJr(0, 0, 9);
	
	afficherCorbeau(10, 2);
	afficherCorbeau(16, 1);
	
	effacerCarres(11, 9, 2, 2);

	afficherEchec(1);
	afficherScore(1999);*/

int main(int argc, char* argv[])
{
	ouvrirFenetreGraphique();

	sigset_t mask;
	sigfillset(&mask);
	sigprocmask(SIG_BLOCK,&mask,NULL);

	// -------- Initialisation des mutex ----------------
	pthread_mutex_init(&mutexEvenement,NULL);
	pthread_mutex_init(&mutexGrilleJeu,NULL);
	pthread_mutex_init(&mutexScore,NULL);
	pthread_mutex_init(&mutexDK,NULL);
	// -------------------------------------------------

	// -------- Initialisation des cond ----------------
	pthread_cond_init(&condDK,NULL);
	pthread_cond_init(&condScore,NULL);
	// -------------------------------------------------

	// -------- Creation des Cles ----------------
	pthread_key_create(&keySpec,NULL);
	// -------------------------------------------------

	pthread_create(&threadCle,NULL,ThreadCle,NULL);
	pthread_create(&threadEvenements,NULL,ThreadEvent,NULL);
	pthread_create(&threadDK,NULL,ThreadDK,NULL);
	pthread_create(&threadScore,NULL,ThreadScore,NULL);
	pthread_create(&threadEnnemis,NULL,ThreadEnnemis,NULL);
	
	do{
		pthread_create(&threadDKJr,NULL,ThreadDKJr,NULL);
		pthread_join(threadDKJr,NULL);

		nbrVie++;

		afficherEchec(nbrVie);
	}while(nbrVie < 3);

	pthread_join(threadEvenements,NULL);

	pthread_key_delete(keySpec);

	return 0;
}

// -------------------------------------

void initGrilleJeu()
{
  int i, j;   
  
  pthread_mutex_lock(&mutexGrilleJeu);

  for(i = 0; i < 4; i++)
    for(j = 0; j < 7; j++)
      setGrilleJeu(i, j);

  pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
  grilleJeu[l][c].type = type;
  grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{   
   for(int j = 0; j < 4; j++)
   {
       for(int k = 0; k < 8; k++)
          printf("%d  ", grilleJeu[j][k].type);
       printf("\n");
   }

   printf("\n");   
}

// ------------------------------------------------------------------------


void* ThreadCle(void*){
	struct timespec time = timespec{0,700'000'000};
	int frame = 1;
	bool keydirection = true;

	while(1){
		switch (frame)
		{
			case 1:
				pthread_mutex_lock(&mutexGrilleJeu);
					grilleJeu[0][1].type = CLE;
					fprintf(stdout,"\033[92mCLE DISPONIBLE...\033[0m\n");	
				pthread_mutex_unlock(&mutexGrilleJeu);
			break;
			
			default:
				if(grilleJeu[0][1].type == CLE){
					pthread_mutex_lock(&mutexGrilleJeu);
						grilleJeu[0][1].type = VIDE;
						fprintf(stdout,"\033[91mCLE INDISPONIBLE...\033[0m\n");			
					pthread_mutex_unlock(&mutexGrilleJeu);
				}
			break;
		}
		
		effacerCarres(3,12,2,3);
		afficherCle(frame);

		if(keydirection == true)
			frame++;
		else
			frame--;
		
		if(frame == 4)
			keydirection = false;
		else if(frame == 1)
			keydirection = true;			

		nanosleep(&time,NULL);
	}
}


void* ThreadEvent(void*){

	int evt;
	struct timespec time = timespec{0,100'000'000};

	while (1)
	{		
		evt = lireEvenement();

		if(evt == SDL_QUIT)
			exit(0);
		/*else if(evt == SDLK_UP && nbrVie != 3)
			fprintf(stdout,"Key = SDLK_UP\n");
		else if(evt == SDLK_DOWN && nbrVie != 3)
			fprintf(stdout,"Key = SDLK_DOWN\n");
		else if(evt == SDLK_LEFT && nbrVie != 3)
			fprintf(stdout,"Key = SDLK_LEFT\n");
		else if(evt == SDLK_RIGHT && nbrVie != 3)
			fprintf(stdout,"Key = SDLK_RIGHT\n");*/

		pthread_mutex_lock(&mutexEvenement);
			evenement = evt;
		pthread_mutex_unlock(&mutexEvenement);


		//fprintf(stdout,"ENVOIE SIGNAL A THREADDKJr DKJr\n");
		pthread_kill(threadDKJr,SIGQUIT);
		//fprintf(stdout,"FIN SIGNAL A THREADDKJr DKJr\n");

		nanosleep(&time,NULL);
		pthread_mutex_lock(&mutexEvenement);
			evenement = AUCUN_EVENEMENT;
		pthread_mutex_unlock(&mutexEvenement);
	}
}

void* ThreadScore(void*){
	while(1){
		pthread_mutex_lock(&mutexScore);
			pthread_cond_wait(&condScore,&mutexScore);
			if(MAJScore)
			{
				MAJScore = false;
				afficherScore(score);
			}
		pthread_mutex_unlock(&mutexScore);
	}
	pthread_exit(NULL);
}


void* ThreadDK(void*){

	int NrPieceCage;
	struct timespec wait = {0,700'000'000};

	while(1){
		NrPieceCage = 1;
		afficherCage(1);
		afficherCage(2);
		afficherCage(3);
		afficherCage(4);

		while(NrPieceCage <= 4)
		{
			pthread_mutex_lock(&mutexDK);
			pthread_cond_wait(&condDK,&mutexDK);
			if(MAJDK)
			{
				MAJDK = false;
				switch(NrPieceCage)
				{
					case 1:
						effacerCarres(2,7,2,2);
					break;

					case 2:
						effacerCarres(2,9,2,2);
					break;

					case 3:
						effacerCarres(4,7,2,2);
					break;

					case 4:
						effacerCarres(4,9,2,2);
					break;
				}
				NrPieceCage++;
			}
			pthread_mutex_unlock(&mutexDK);
		}

		afficherRireDK();
		nanosleep(&wait,NULL);
		effacerCarres(3,8,2,2);

		// ---------------------------------
		// Modification des valeurs de score
		pthread_mutex_lock(&mutexScore);
			score += 10;
			MAJScore = true;
		pthread_mutex_unlock(&mutexScore);
		pthread_cond_signal(&condScore);
		// ---------------------------------
	}

	pthread_exit(NULL);
}


void* ThreadDKJr(void*){

	//fprintf(stdout,"Creation THREAD DKJr\n");

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);
	sigaddset(&mask,SIGINT);
	pthread_sigmask(SIG_UNBLOCK,&mask,NULL);

	struct sigaction signal;

	sigemptyset(&signal.sa_mask);
	signal.sa_flags = 0;
	signal.sa_handler = HandlerSIGQUIT;
	sigaction(SIGQUIT,&signal,NULL);

	sigemptyset(&signal.sa_mask);
	signal.sa_flags = 0;
	signal.sa_handler = HandlerSIGINT;
	sigaction(SIGINT,&signal,NULL);

	pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(3,1,DKJR,pthread_self());
		afficherDKJr(11, 9, 1);
		etatDKJr = LIBRE_BAS;
		positionDKJr = 1;
	pthread_mutex_unlock(&mutexGrilleJeu);


	bool on = true;

	while(on){
		pause();

		pthread_mutex_lock(&mutexEvenement);
		pthread_mutex_lock(&mutexGrilleJeu);

		if(etatDKJr == LIBRE_BAS){
			switch(evenement){
				case SDLK_LEFT:
					if(positionDKJr > 1){
						setGrilleJeu(3,positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

						positionDKJr--;

						setGrilleJeu(3,positionDKJr,DKJR,pthread_self());
						afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
					}
				break;

				case SDLK_RIGHT:
					if(positionDKJr < 7){
							setGrilleJeu(3,positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

							positionDKJr++;

							setGrilleJeu(3,positionDKJr,DKJR,pthread_self());
							afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
				break;

				case SDLK_UP:
					struct timespec time = timespec{1,400'000'000};

					setGrilleJeu(3,positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					
					if(grilleJeu[2][positionDKJr].type != VIDE)
					{
						pthread_kill(grilleJeu[2][positionDKJr].tid,SIGUSR1);

						pthread_mutex_unlock(&mutexGrilleJeu);
						pthread_mutex_unlock(&mutexEvenement);
						pthread_exit(NULL);
					}
					if(positionDKJr == 1 || positionDKJr == 5){
						setGrilleJeu(2,positionDKJr,DKJR,pthread_self());
						afficherDKJr(10,(positionDKJr * 2) + 7,7);
						etatDKJr = LIANE_BAS;
					}
					else if(positionDKJr == 7){ // si Dkjr n'est pas à un emplacement de lianne alors il ne change pas d'etat
						setGrilleJeu(2,positionDKJr,DKJR,pthread_self());
						afficherDKJr(10,(positionDKJr * 2) + 7,5);						
						etatDKJr = DOUBLE_LIANE_BAS;
					}
					else
					{
						setGrilleJeu(2,positionDKJr,DKJR,pthread_self());
						afficherDKJr(10,(positionDKJr * 2) + 7,8);
						//printGrille();

						// mise en place de l'attente de 1,4 seconde.
						pthread_mutex_unlock(&mutexGrilleJeu);

						if(nanosleep(&time,NULL) == 0)
							printf("\033[93mFin Normale sleep\n\033[0m");
					
						pthread_mutex_lock(&mutexGrilleJeu);

						setGrilleJeu(2,positionDKJr);
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(3,positionDKJr,DKJR,pthread_self());
						afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
					}
				break;
			}
		} 
		else if(etatDKJr == LIANE_BAS){
			if(evenement == SDLK_DOWN)
			{
				setGrilleJeu(2,positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

				setGrilleJeu(3,positionDKJr,DKJR,pthread_self());
				afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);

				etatDKJr = LIBRE_BAS;
			}

		}
		else if(etatDKJr == DOUBLE_LIANE_BAS){
			if(evenement == SDLK_DOWN){

				setGrilleJeu(2,positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

				setGrilleJeu(3,positionDKJr,DKJR,pthread_self());
				afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);

				etatDKJr = LIBRE_BAS;				
			}
			else if(evenement == SDLK_UP){
				setGrilleJeu(2,positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

				setGrilleJeu(1,positionDKJr,DKJR,pthread_self());
				afficherDKJr(7,(positionDKJr * 2) + 7,6);

				etatDKJr = LIBRE_HAUT;	
			}
		}
		else if(etatDKJr == LIBRE_HAUT){
			switch(evenement){
				case SDLK_LEFT:
					if(positionDKJr >= 3){
						struct timespec time = timespec{0,500'000'000};
						struct timespec time2 = timespec{0,250'000'000};
						setGrilleJeu(1,positionDKJr);
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

						positionDKJr--;
						if(positionDKJr == 2 && grilleJeu[0][1].type == CLE){ // probablement liberer les mutexes
							fprintf(stdout,"\033[96mDKJr a recupere une cle...\033[0m\n");
							afficherDKJr(6,11,9);
							
							nanosleep(&time,NULL);

							effacerCarres(5, 12, 3, 2);
							afficherDKJr(6,11,10);

							nanosleep(&time,NULL);

							effacerCarres(3, 11, 3, 2);
							afficherDKJr(6,10,11);
							
							// ---------------------------------
							// Envoie D'un signal 
							// au thread DK
							pthread_mutex_lock(&mutexDK);
								MAJDK = true;
							pthread_mutex_unlock(&mutexDK);
							pthread_cond_signal(&condDK);
							// ---------------------------------

							// ---------------------------------
							// Modification des valeurs de score
							pthread_mutex_lock(&mutexScore);
								score += 10;
								MAJScore = true;
							pthread_mutex_unlock(&mutexScore);
							pthread_cond_signal(&condScore);
							// ---------------------------------

							nanosleep(&time,NULL);

							effacerCarres(6, 10, 2, 3);

							setGrilleJeu(3,1,DKJR,pthread_self());
							afficherDKJr(11, 9, 1);
							etatDKJr = LIBRE_BAS;
							positionDKJr = 1;
						}
						else if(positionDKJr == 2 && grilleJeu[0][1].type == VIDE){ // probablement liberer les mutexes
							fprintf(stdout,"\033[95mDKJr a ratee la cle, -1 vie...\033[0m\n");
							afficherDKJr(5,11,9);

							nanosleep(&time,NULL);

							effacerCarres(5, 11, 3, 3);
							afficherDKJr(6,(positionDKJr * 2) + 7,12);

							nanosleep(&time,NULL);
							// affichage buisson
							effacerCarres(6, (positionDKJr * 2) + 7, 3, 2);
							afficherDKJr(11,0 + 7,13);

							//disparition
							nanosleep(&time2,NULL);
							effacerCarres(11, 7, 2, 2);

							//reaparition
							nanosleep(&time2,NULL);
							afficherDKJr(12,0 + 7,13);

							//disparition
							nanosleep(&time2,NULL);
							effacerCarres(11, 7, 2, 2);

							pthread_mutex_unlock(&mutexEvenement);
							pthread_mutex_unlock(&mutexGrilleJeu);
							
							pthread_exit(0);
						}
						else{							
							setGrilleJeu(1,positionDKJr,DKJR,pthread_self());
							afficherDKJr(7,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
					}
				break;

				case SDLK_RIGHT:
					if(positionDKJr < 7){
							setGrilleJeu(1,positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

							positionDKJr++;
							setGrilleJeu(1,positionDKJr,DKJR,pthread_self());
							if(positionDKJr == 7)
							{
								afficherDKJr(7,(positionDKJr * 2) + 7,6);
							}
							else
								afficherDKJr(7,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
				break;

				case SDLK_DOWN :

					if(positionDKJr == 7){
						setGrilleJeu(1,positionDKJr);
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(2,positionDKJr,DKJR,pthread_self());
						afficherDKJr(10,(positionDKJr * 2) + 7,5);

						etatDKJr = DOUBLE_LIANE_BAS;
					}
					
					
				break;

				case SDLK_UP:
					struct timespec time = timespec{1,400'000'000};

					/* ------------- verison caca ------------
					if(positionDKJr != 2 &&positionDKJr != 7){						
						//setGrilleJeu(1,positionDKJr);	
						setGrilleJeu(0,positionDKJr,DKJR,pthread_self());
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
					}
					-----------------------------------------*/
					setGrilleJeu(1,positionDKJr);	
					if(positionDKJr == 6){
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(0,positionDKJr,DKJR,pthread_self());
						afficherDKJr(6,(positionDKJr * 2) + 7,7);
						etatDKJr = LIANE_HAUT;
					}
					else if(positionDKJr != 2 &&positionDKJr != 7)
					{
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(0,positionDKJr,DKJR,pthread_self());
						afficherDKJr(6,(positionDKJr * 2) + 7,8);
						//printGrille();

						// mise en place de l'attente de 1,4 seconde.
						pthread_mutex_unlock(&mutexGrilleJeu);

						if(nanosleep(&time,NULL) == 0)
							printf("\033[93mFin Normale sleep\n\033[0m");
					
						pthread_mutex_lock(&mutexGrilleJeu);

						setGrilleJeu(0,positionDKJr);
						effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(1,positionDKJr,DKJR,pthread_self());
						afficherDKJr(7,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
					}
				break;
			}

		}
		else if(etatDKJr == LIANE_HAUT){
			if(evenement == SDLK_DOWN)
			{
				setGrilleJeu(0,positionDKJr);
				effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);

				setGrilleJeu(1,positionDKJr,DKJR,pthread_self());
				afficherDKJr(7,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);

				etatDKJr = LIBRE_HAUT;
			}
		}		

		//printGrille();

		pthread_mutex_unlock(&mutexEvenement);
		pthread_mutex_unlock(&mutexGrilleJeu);		
	}

	pthread_exit(0);
}


void* ThreadEnnemis(void*){

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGALRM);
	pthread_sigmask(SIG_UNBLOCK,&mask,NULL);

	struct sigaction signal;

	sigemptyset(&signal.sa_mask);
	signal.sa_flags = 0;
	signal.sa_handler = HandlerSIGALRM;
	sigaction(SIGALRM,&signal,NULL);

	struct timespec wait = {4,0};

	srand(time(NULL));

	alarm(15);

	while(1){
		int x = rand()%2+1;
		if(x == 1){
			pthread_t Croco;
			pthread_create(&Croco,NULL,ThreadCroco,NULL);
			printf("\033[38;5;107mCROCO CREATED\033[0m\n");
		}
		else{
			pthread_t Corback;
			pthread_create(&Corback,NULL,ThreadCroco,NULL);
			printf("\033[38;5;99mCORBACK CREATED\033[0m\n");
		}
		wait.tv_sec = delaiEnnemis/1000;
		wait.tv_nsec = (delaiEnnemis%1000)*1'000'000;
		nanosleep(&wait,NULL);
	}
	pthread_exit(NULL);
}
void* ThreadCorbeau(void*){

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK,&mask,NULL);

	struct sigaction signal;

	sigemptyset(&signal.sa_mask);
	signal.sa_flags = 0;
	signal.sa_handler = HandlerSIGUSR1;
	sigaction(SIGUSR1,&signal,NULL);


	struct timespec wait = {0,700'000'000};
	int* PosHorCourrante = new int(0);
	pthread_setspecific(keySpec,PosHorCourrante);

	while(*PosHorCourrante < 8){

		pthread_mutex_lock(&mutexGrilleJeu);
			afficherCorbeau(*PosHorCourrante*2+8,(*PosHorCourrante)%2+1);
			setGrilleJeu(2,*PosHorCourrante,CORBEAU,pthread_self());
		pthread_mutex_unlock(&mutexGrilleJeu);

		nanosleep(&wait,NULL);

		pthread_mutex_lock(&mutexGrilleJeu);
			effacerCarres(9,*PosHorCourrante*2+8,2,1);
			setGrilleJeu(2,*PosHorCourrante);
		pthread_mutex_unlock(&mutexGrilleJeu);
		
		pthread_mutex_lock(&mutexGrilleJeu);
		if(grilleJeu[2][(*PosHorCourrante)+1].type == DKJR)
		{
			pthread_kill(grilleJeu[2][(*PosHorCourrante)+1].tid,SIGINT);

			effacerCarres(9,*PosHorCourrante*2+8,2,1);
			setGrilleJeu(2,*PosHorCourrante);

			pthread_mutex_unlock(&mutexGrilleJeu);

			delete PosHorCourrante;
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&mutexGrilleJeu);

		(*PosHorCourrante)++;
	}

	delete PosHorCourrante;
	pthread_exit(NULL);
}
void* ThreadCroco(void*){

	struct timespec wait = {0,700'000'000};
	S_CROCO* PosHorCourrante = new S_CROCO{true,2};
	pthread_setspecific(keySpec,PosHorCourrante);


	while(PosHorCourrante->haut){
		if(PosHorCourrante->position < 8){
			pthread_mutex_lock(&mutexGrilleJeu);
				afficherCroco(PosHorCourrante->position*2+7,PosHorCourrante->position%2+1);
				setGrilleJeu(1,PosHorCourrante->position,CROCO,pthread_self());
			pthread_mutex_unlock(&mutexGrilleJeu);

			nanosleep(&wait,NULL);

			pthread_mutex_lock(&mutexGrilleJeu);
				effacerCarres(8,PosHorCourrante->position*2+7,1,1);
				setGrilleJeu(1,PosHorCourrante->position);
			pthread_mutex_unlock(&mutexGrilleJeu);

			(PosHorCourrante->position)++;

			if(PosHorCourrante->position == 8){
				pthread_mutex_lock(&mutexGrilleJeu);
					afficherCroco(PosHorCourrante->position*2+7,3);
				pthread_mutex_unlock(&mutexGrilleJeu);

				nanosleep(&wait,NULL);

				pthread_mutex_lock(&mutexGrilleJeu);
					effacerCarres(9,PosHorCourrante->position*2+7,1,1);
				pthread_mutex_unlock(&mutexGrilleJeu);

				PosHorCourrante->position = 7;
				PosHorCourrante->haut = false;
			}
		}
	}

	while(!PosHorCourrante->haut){
		if(PosHorCourrante->position > 0){
			pthread_mutex_lock(&mutexGrilleJeu);
				afficherCroco(PosHorCourrante->position*2+8,PosHorCourrante->position%2+1+3);
				setGrilleJeu(3,PosHorCourrante->position,CROCO,pthread_self());
			pthread_mutex_unlock(&mutexGrilleJeu);

			nanosleep(&wait,NULL);

			pthread_mutex_lock(&mutexGrilleJeu);
				effacerCarres(12,PosHorCourrante->position*2+8,1,1);
				setGrilleJeu(3,PosHorCourrante->position);
			pthread_mutex_unlock(&mutexGrilleJeu);

			(PosHorCourrante->position)--;
		}
		else{
			delete PosHorCourrante;
			pthread_exit(NULL);
		}
	}

	pthread_exit(NULL);
}



 //------------------------------------------------------

 void printGrille(){

	for (int  i = 0; i < 4; i++)
	{
		for(int j = 0; j < 8; j++){
			printf("%4d ",grilleJeu[i][j]);
			fflush(stdout);
		}
			
		printf("\n");
	}
	printf("\n");
 }

 //------------------------------------------------------

void HandlerSIGALRM(int){
	delaiEnnemis -= 250;
	if(delaiEnnemis > 2500)
		alarm(15);
	printf("\033[38;5;212mDelay = %d\033[0m\n",delaiEnnemis);
}

void HandlerSIGUSR1(int){
	int* Pos = (int*)pthread_getspecific(keySpec);

	pthread_mutex_lock(&mutexGrilleJeu);
			effacerCarres(9,*Pos*2+8,2,1);
			setGrilleJeu(2,*Pos);
	pthread_mutex_unlock(&mutexGrilleJeu);

	delete Pos;
	pthread_exit(NULL);
}

void HandlerSIGQUIT(int){
	//fprintf(stdout,"HandlerSIQUIT\n");
}

void HandlerSIGINT(int){

	pthread_mutex_lock(&mutexGrilleJeu);
	if(etatDKJr == LIBRE_BAS){
		setGrilleJeu(2,positionDKJr);
		effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
		pthread_mutex_unlock(&mutexEvenement);
	}
	else if(etatDKJr == LIANE_BAS){
		setGrilleJeu(2,positionDKJr);
		effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
	}
	else{
		setGrilleJeu(2,positionDKJr);
		effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
	}

	pthread_mutex_unlock(&mutexGrilleJeu);
	pthread_exit(NULL);
}