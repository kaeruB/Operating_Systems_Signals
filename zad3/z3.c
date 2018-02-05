#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t rodzicPID;
pid_t procesPotomny;
int sentToChild = 0;
int gotByParent = 0;
int gotByChild = 0;
int L, Type;
int end = 0;


void handleSIGINT(int sig){
	printf("\n");
	if(sig == SIGINT) {
		printf("\n\nOdebrano sygnal SIGINT\n\n");
		kill(procesPotomny, 9);				//tych nie trzeba liczyc?
		exit(1);
	}
}
//______________________________________________________________________________________________
void sendSIGUSR1 (pid_t pid) {
	if (Type == 1){
		kill(pid, SIGUSR1);
	}
	if (Type == 2){
		const union sigval sv;
		sigqueue(pid, SIGUSR1, sv);
	}
	if (Type ==3){
		kill(pid, SIGRTMIN+2);
	}
}

void sigusr1Handler(int sig, siginfo_t *sigInfo, void* taisetsuJiaNai){
	//jesli wysylajacy to rodzic i sygnalem jest SIGUSR1 to:
	if (sigInfo->si_pid == rodzicPID){
		//odeslij do rodzica
		//printf("\nDziecko dostaje USR1\n");
		gotByChild++;
		sendSIGUSR1(rodzicPID);
		//kill(rodzicPID, SIGUSR1);
	}
	else {
		//printf("\nRodzic dostaje sygnal\n");
		gotByParent++;
		return;
	}
}


//__________________________________________________________________________________________________
void sigusr2Handler(int sig){
	end = 1;
	return;
}


void sendSIGUSR2(int pid){
	if (Type == 1){
		kill(pid, SIGUSR2);
	}
	if (Type == 2){
		const union sigval sv;
		sigqueue(pid, SIGUSR2, sv);
	}
	if (Type ==3){
		kill(pid, SIGRTMIN+8);
	}
}

//__________________________________________________________________________________________________
int main (int argc, char *argv[]) 
{
	int pipeT[2];
	int w;

	if (argc != 3 || (atoi(argv[2]) != 1 && atoi(argv[2]) != 2 && atoi(argv[2]) != 3)){
		printf("\nZle argumenty\n[ile sygnalow] [Typ]:\n1 - kill (SIGUSR1 i -2),\n2 - sigqueue,\n3 - kill (sygnaly czasu rzeczywistego)\n");
		exit(1);
	} 
	L = atoi(argv[1]);
	Type = atoi(argv[2]);

	rodzicPID = getpid();

	//lapanie sygnalow od rodzica przez proces potomny
	signal(SIGINT, handleSIGINT);

	struct sigaction sigusr1Struct;
	sigusr1Struct.sa_sigaction = *sigusr1Handler;
	sigusr1Struct.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sigusr1Struct, NULL);

	signal(SIGUSR2, sigusr2Handler);

	sigaction(SIGRTMIN+2, &sigusr1Struct, NULL);
	signal(SIGRTMIN+8, sigusr2Handler);

	pipe(pipeT);
	
	
	procesPotomny = fork();
	if (procesPotomny == -1){
		printf("\nNie udalo sie utworzyc procesu potomnego.\n");
		exit(1);
	}
	//child
	if (procesPotomny == 0){
		while(!end) pause();
		close(pipeT[0]);
		write(pipeT[1], &gotByChild, sizeof(int));
		return(0);				
	}
	//rodzic, wyslanie L sygnalow do procesu potomnego
	else if(procesPotomny > 0){
		for(int i = 0; i < L; i++){
			sendSIGUSR1(procesPotomny);
			usleep(10);
			sentToChild++;
		}
		sendSIGUSR2(procesPotomny);
		wait(&w);
		close(pipeT[1]);
		read(pipeT[0], &gotByChild, sizeof(int));	
		printf("\nWyslane do potomka: %d", sentToChild);
		printf("\nOdebrane przez potomka: %d", gotByChild);
		printf("\nOdebrane przez rodzica: %d\n", gotByParent);
	}	
	
	return(0);
}

