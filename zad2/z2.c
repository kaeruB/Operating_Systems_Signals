#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int numberOfSignals = 0;
int N, K;
pid_t* procesyPotomne;
int* usr1signals;
int numberOfRTSignals = 0;

//int
void sigintHandler(int sig){
	printf("\nZabicie prcesow potomnych\n");
	for (int i = 0; i < N; i++)
		kill(procesyPotomne[i], SIGKILL);
	free(procesyPotomne);
	free(usr1signals);
	exit(1);
}

//rt
//do odbierania SIGRT od potomkow, potomek konczy dzialanie
void rtSignalHandler(int signo, siginfo_t *si, void *taisetsuJiaNai){
	numberOfRTSignals++;
	int wStatus;
	waitpid(si->si_pid, &wStatus, 0); //0-czekaj na jakikolwiek proces potomny
	printf("PID procesu potomnego: %d, SIGRT%d Czas: %d\n", si->si_pid, signo, WEXITSTATUS(wStatus));
	usleep(100);
	if(numberOfRTSignals == N){
		free(procesyPotomne);
		free(usr1signals);
		exit(0);
	}
}

//usr2
void sigusr2Handler(int sig){
	//proces potomny otrzymuje sygnal => zostaje wybudzony	(wczesniej trzeba uspic sleepem)
	//printf("%d got SIGUSR2\n", getpid()); 
}

//usr1
void usr1SignalHandler(int signo, siginfo_t *si, void *taisetsuJiaNai){
	numberOfSignals++;
	//printf("%d got SIGUSR1 from %d\n", getpid(), si->si_pid); 

	int i;
	for (i = 0; i < N; i++){
		if(procesyPotomne[i] == si->si_pid)
			usr1signals[i] = 1;
	}

	if(numberOfSignals == K){
	//printf("puszcza\n");
		for(i = 0; i < N; i++)
			if(usr1signals[i] != 0)
				kill(procesyPotomne[i], SIGUSR2);
	}

	if(numberOfSignals > K){
		//printf("natychmiast: ");
		kill(si->si_pid, SIGUSR2);
	}

}



int main (int argc, char *argv[]) 
{
	struct sigaction rtStruct;
	struct sigaction usr1Struct;
	pid_t procesMacierzysty = getpid();
	int randomRTsig;
	int randomTime;
	time_t start, stop;
	

//	srand(time(NULL));

	if (argc != 3 || atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0 || atoi(argv[1]) < atoi(argv[2])){
		printf("\nZle argumenty\n[ile potomkow] [ile prosb]\n");
		exit(1);
	} 
	N = atoi(argv[1]);
	K = atoi(argv[2]);

	
	
	
	//rt
	rtStruct.sa_flags = SA_SIGINFO;
	rtStruct.sa_sigaction = *rtSignalHandler;
	for (int no = SIGRTMIN; no <= SIGRTMAX; no++)
		sigaction(no, &rtStruct, NULL);
	
	//usr2 - do wybudzania procesu potomnego
	signal(SIGUSR2, sigusr2Handler);

	//int
	signal(SIGINT, sigintHandler);
	
	//usr1
	usr1Struct.sa_flags = SA_SIGINFO;
	usr1Struct.sa_sigaction = *usr1SignalHandler;
	sigaction(SIGUSR1, &usr1Struct, NULL);

	procesyPotomne = calloc(N, sizeof(pid_t));
	usr1signals = calloc(N, sizeof(int));

	if(procesyPotomne == NULL || usr1signals == NULL){
		printf("\nBlad alokacji pamieci\n");
		exit(1);
	}
	

	//tworzenie potomkow
	for (int i = 0; i < N; i++){
		procesyPotomne[i] = fork();
		//printf("%d--", procesyPotomne[i]);
		if(procesyPotomne[i] == 0) break; //jak jest procesem potomnym, to ma nie tworzyc nowych
	}

	int czyPotomny = 0;
	for (int j = 0; j < N; j++) {
		if (procesyPotomne[j] == 0){
			czyPotomny = 1;
			break;
		}
	}

		
	if(czyPotomny){
	//proces potomny
		srand(getpid()-procesMacierzysty);
		//printf("PROCES POTOMNY %d\n", getpid());
		//zasniecie na kilka s
		randomTime =rand() % (100000 + 1 - 10000) + 10000;
		usleep(randomTime * 100);	//usleep w mikrosekundach

		//wyslanie usr1
		start = time(NULL);
		//printf("wysylam usr1\n");
		kill(procesMacierzysty, SIGUSR1);
		pause();

		//wyslanie randomowego sygnalu od rtmin do rtmax
		//srand(time(NULL));
		randomRTsig = rand()%(SIGRTMAX + 1  - SIGRTMIN) + SIGRTMIN;
		kill(procesMacierzysty, randomRTsig);
		stop = time(NULL);

		return (int) (stop - start);			
	}
		
	else {
		//printf("macierzysty\n");
		while (numberOfRTSignals < N)
			pause();
		pause();

		//printf("koniec\n\n\n");
		free(procesyPotomne);
		free(usr1signals);

		return 0;
	}	
		
	
}


