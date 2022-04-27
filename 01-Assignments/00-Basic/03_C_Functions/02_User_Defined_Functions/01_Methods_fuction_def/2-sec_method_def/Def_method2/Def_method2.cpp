#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	int add(void);

	int ires;

	ires = add();

	printf("Sum = %d \n\n", ires);

	return 0;
}

int add(void)
{
	int no1, no2, isum;

	printf("Enter no1\n\n");
	scanf_s("%d", &no1);

	printf("Enter no2\n\n");
	scanf_s("%d", &no2);

	isum = no1 + no2;

	return(isum);
}