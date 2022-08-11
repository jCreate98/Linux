#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char* argv[]){
	int tonum;
	int i;
	int sum = 0;

	if(argc != 2)
	{
		fprintf(stderr, "Unexpected Input\n");
	}

	tonum = atoi(argv[1]);

	for(i = 1; i <= tonum; i++)
	{
		sum += i;	
	}

	printf("Pid: %d, sum (1 ~ %d): %d\n", getpid(), tonum, sum);

	return 0;
}
