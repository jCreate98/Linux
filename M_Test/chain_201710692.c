#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	int i;
	int childpid;
	int status;

	for(i = 0; i < argc-1; i++)
	{
		childpid = fork();
		if(childpid != 0)
			break;
		else continue;
	}

	if(childpid == 0)
	{
		execl("./sum_201710692", "sum_201710692", argv[i], NULL);
		fprintf(stderr, "EXEL Failed\n");
	}
	else if(childpid == -1)
	{
		fprintf(stderr, "Fork Failed\n");
	}
	else
	{	
		if(i == 0){
			wait(&status);
			printf("Main pid: %d, all children done! \n", getpid());
		}
		else {
			wait(&status);
			execl("./sum_201710692","sum_201710692", argv[i], NULL);
			fprintf(stderr, "EXEL Failed\n");
		}
	}
	return 0;
}
