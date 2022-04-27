#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	void add(void);

	add();
	return 0;
}

void add(void)
{
	int no1, no2, isum;

	printf("Enter no1\n\n");
	scanf_s("%d", &no1);

	printf("Enter no2\n\n");
	scanf_s("%d", &no2);

	isum = no1 + no2;

	printf("Sum = %d \n\n",isum);
}