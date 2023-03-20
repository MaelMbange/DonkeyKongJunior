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

// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	ouvrirFenetreGraphique();

	sigset_t mask;
	sigfillset(&mask);
	sigprocmask(SIG_BLOCK,&mask,NULL);

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

	pthread_create(&threadCle,NULL,ThreadCle,NULL);
	pthread_create(&threadDKJr,NULL,ThreadDKJr,NULL);
	pthread_create(&threadEvenements,NULL,ThreadEvent,NULL);
	

	pthread_join(threadEvenements,NULL);
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
		else if(evt == SDLK_UP)
			fprintf(stdout,"Key = SDLK_UP\n");
		else if(evt == SDLK_DOWN)
			fprintf(stdout,"Key = SDLK_DOWN\n");
		else if(evt == SDLK_LEFT)
			fprintf(stdout,"Key = SDLK_LEFT\n");
		else if(evt == SDLK_RIGHT)
			fprintf(stdout,"Key = SDLK_RIGHT\n");

		pthread_mutex_lock(&mutexEvenement);
			evenement = evt;
		pthread_mutex_unlock(&mutexEvenement);

		pthread_kill(threadDKJr,SIGQUIT);

		nanosleep(&time,NULL);
		pthread_mutex_lock(&mutexEvenement);
			evenement = AUCUN_EVENEMENT;
		pthread_mutex_unlock(&mutexEvenement);
	}
}


void* ThreadDKJr(void*){

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);
	pthread_sigmask(SIG_UNBLOCK,&mask,NULL);

	struct sigaction signal;

	sigemptyset(&signal.sa_mask);
	signal.sa_flags = 0;
	signal.sa_handler = HandlerSIGQUIT;
	sigaction(SIGQUIT,&signal,NULL);

	pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(3,1,DKJR);
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

						setGrilleJeu(3,positionDKJr,DKJR);
						afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
					}
				break;

				case SDLK_RIGHT:
					if(positionDKJr < 7){
							setGrilleJeu(3,positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

							positionDKJr++;

							setGrilleJeu(3,positionDKJr,DKJR);
							afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
				break;

				case SDLK_UP:
					struct timespec time = timespec{1,400'000'000};

					setGrilleJeu(3,positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					
					setGrilleJeu(2,positionDKJr,DKJR);
					if(positionDKJr == 1 || positionDKJr == 5){
						afficherDKJr(10,(positionDKJr * 2) + 7,7);
						etatDKJr = LIANE_BAS;
					}
					else if(positionDKJr == 7){ // si Dkjr n'est pas à un emplacement de lianne alors il ne change pas d'etat
						afficherDKJr(10,(positionDKJr * 2) + 7,5);						
						etatDKJr = DOUBLE_LIANE_BAS;
					}
					else
					{
						afficherDKJr(10,(positionDKJr * 2) + 7,8);

						// mise en place de l'attente de 1,4 seconde.
						pthread_mutex_unlock(&mutexGrilleJeu);

						if(nanosleep(&time,NULL) == 0)
							printf("\033[93mFin Normale sleep\n\033[0m");
					
						pthread_mutex_lock(&mutexGrilleJeu);

						setGrilleJeu(2,positionDKJr);
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(3,positionDKJr,DKJR);
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

				setGrilleJeu(3,positionDKJr,DKJR);
				afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);

				etatDKJr = LIBRE_BAS;
			}

		}
		else if(etatDKJr == DOUBLE_LIANE_BAS){
			if(evenement == SDLK_DOWN){

				setGrilleJeu(2,positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

				setGrilleJeu(3,positionDKJr,DKJR);
				afficherDKJr(11,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);

				etatDKJr = LIBRE_BAS;				
			}
			else if(evenement == SDLK_UP){
				setGrilleJeu(2,positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

				setGrilleJeu(1,positionDKJr,DKJR);
				afficherDKJr(7,(positionDKJr * 2) + 7,6);

				etatDKJr = LIBRE_HAUT;	
			}
		}
		else if(etatDKJr == LIBRE_HAUT){
			switch(evenement){
				case SDLK_LEFT:
					if(positionDKJr >= 3){
						setGrilleJeu(1,positionDKJr);
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

						positionDKJr--;
						if(positionDKJr == 2 && grilleJeu[0][1].type == CLE){ // probablement liberer les mutexes
							fprintf(stdout,"\033[96mDKJr a recupere une cle...\033[0m\n");
						}
						else if(positionDKJr == 2 && grilleJeu[0][1].type == VIDE){ // probablement liberer les mutexes
							fprintf(stdout,"\033[95mDKJr a ratee la cle, -1 vie...\033[0m\n");
						}
						else{							
							setGrilleJeu(1,positionDKJr,DKJR);
							afficherDKJr(7,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
					}
				break;

				case SDLK_RIGHT:
					if(positionDKJr < 7){
							setGrilleJeu(1,positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

							positionDKJr++;
							setGrilleJeu(1,positionDKJr,DKJR);
							if(positionDKJr == 7)
							{
								afficherDKJr(7,(positionDKJr * 2) + 7,6);
							}
							else
								afficherDKJr(7,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
				break;

				case SDLK_DOWN :
					setGrilleJeu(1,positionDKJr);
					effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

					setGrilleJeu(1,positionDKJr);
					afficherDKJr(10,(positionDKJr * 2) + 7,5);

					etatDKJr = DOUBLE_LIANE_BAS;
					
				break;

				case SDLK_UP:
					struct timespec time = timespec{1,400'000'000};

					if(positionDKJr != 2 &&positionDKJr != 7){							
						setGrilleJeu(1,positionDKJr);
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
					}
					
					if(positionDKJr == 6){
						afficherDKJr(6,(positionDKJr * 2) + 7,7);
						etatDKJr = LIANE_HAUT;
					}
					else if(positionDKJr != 2 &&positionDKJr != 7)
					{
						afficherDKJr(6,(positionDKJr * 2) + 7,8);

						// mise en place de l'attente de 1,4 seconde.
						pthread_mutex_unlock(&mutexGrilleJeu);

						if(nanosleep(&time,NULL) == 0)
							printf("\033[93mFin Normale sleep\n\033[0m");
					
						pthread_mutex_lock(&mutexGrilleJeu);

						setGrilleJeu(0,positionDKJr);
						effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(1,positionDKJr,DKJR);
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

				setGrilleJeu(1,positionDKJr,DKJR);
				afficherDKJr(7,(positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);

				etatDKJr = LIBRE_HAUT;
			}
		}

		pthread_mutex_unlock(&mutexEvenement);
		pthread_mutex_unlock(&mutexGrilleJeu);		
	}

	pthread_exit(0);
}
 //------------------------------------------------------

 void HandlerSIGQUIT(int){
	 fprintf(stdout,"HandlerSIQUIT\n");
 }