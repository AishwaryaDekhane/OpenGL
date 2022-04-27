#include<stdio.h>

int main(int argc,char* argv[])
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
	return 0;
}