#include<stdio.h>
//#include<ctype.h>
#include<stdlib.h>

int main(int argc, char* argv[], char *envp[])
{
	int i;
	int ino;
	int isum = 0;

	printf("Sum of all int cmd line arguments = \n");

	for (i = 1;i < argc;i++)
	{
		ino = atoi(argv[i]);             //atoi= all cmd arguments other than int are ignored - ASCII to int
		isum = isum + ino;
	}
	printf("Sum= %d\n\n",isum);
	
	return 0;
}