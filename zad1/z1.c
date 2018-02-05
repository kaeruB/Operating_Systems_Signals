#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static const int N = 24;
char alphabet[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'r', 's', 't', 'u', 'w', 'x', 'y', 'z'};
int changeDirection;
int gotSignal;

void print() {
	if (changeDirection == 0){
		for (int i = 0; i < N; i++) {
			if (gotSignal == 1) break;
			printf("%c\n", alphabet[i]);
			sleep(1);
		}
	}
	else if (changeDirection == 1){
		for (int i = N-1; i >= 0; i--) {
			if (gotSignal == 1) break;
			printf("%c\n", alphabet[i]);
			sleep(1);
		}
	}
}

void handleSignal(int sig){
	printf("\n");
	if (sig == SIGINT) {
		printf("\n\nOdebrano sygnal SIGINT\n\n");
		exit(0);
	}
	else if (sig == SIGTSTP) {
		if (changeDirection == 1) changeDirection = 0;
		else if (changeDirection == 0) changeDirection = 1;
		gotSignal = 1;
	}
	
}


int main () 
{
	printf("\n");
	changeDirection = 0;
	gotSignal = 0;
	struct sigaction sigactionStruct;

	signal(SIGTSTP, handleSignal);
	sigactionStruct.sa_handler = handleSignal;
	sigaction(SIGINT, &sigactionStruct, NULL);

	while (1){
		gotSignal = 0;
		print();	
	}

	
	return 0;
}
