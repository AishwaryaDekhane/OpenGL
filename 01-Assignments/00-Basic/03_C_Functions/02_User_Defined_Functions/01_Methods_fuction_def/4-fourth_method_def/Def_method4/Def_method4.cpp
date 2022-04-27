#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	int add(int, int);

	int no1, no2, ires;

	printf("Enter no1\n\n");
	scanf_s("%d", &no1);

	printf("Enter no2\n\n");
	scanf_s("%d", &no2);

	ires = add(no1, no2);


	printf("Sum = %d \n\n", ires);

	return 0;
}

int add(int no1, int no2)
{
	int isum;

	isum = no1 + no2;

	return(isum);
}