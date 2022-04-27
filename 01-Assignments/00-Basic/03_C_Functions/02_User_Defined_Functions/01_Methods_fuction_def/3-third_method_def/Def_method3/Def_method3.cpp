#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	void add(int ,int);

	int no1, no2;

	printf("Enter no1\n\n");
	scanf_s("%d", &no1);

	printf("Enter no2\n\n");
	scanf_s("%d", &no2);

	add(no1,no2);

	return 0;
}

void add(int no1,int no2)
{
	int isum;

	isum = no1 + no2;

	printf("Sum = %d \n\n", isum);
}