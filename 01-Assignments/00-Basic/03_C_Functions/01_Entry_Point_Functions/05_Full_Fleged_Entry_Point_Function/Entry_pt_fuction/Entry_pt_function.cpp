#include<stdio.h>

int main(int argc, char* argv[], char *envp[])
{
	int i;

	printf("Hello Aishwarya Dekhane!!!\n\n");
	printf("No of command line arguments = %d\n\n", argc);

	printf("Command line arguments passed to this program : \n");
	for (i = 0;i < argc;i++)
	{
		printf("Cmd No %d = %s\n", (i + 1), argv[i]);
	}
	printf("\n\n");

	printf("First 10 environmental variables: \n\n");
	for (i = 0;i < 10;i++)
	{
		printf("Cmd No %d = %d\n", (i + 1), envp[i]);
	}
	printf("\n\n");
	return 0;
}