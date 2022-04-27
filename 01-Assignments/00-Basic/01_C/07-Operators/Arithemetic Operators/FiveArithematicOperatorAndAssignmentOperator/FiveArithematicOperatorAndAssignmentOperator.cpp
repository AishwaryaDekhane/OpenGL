#include<stdio.h>

int main()
{
	//variable declartion

	int a;
	int b;
	int result;

	//code

	printf("\nEnter 1 no:");
	scanf_s("%d", &a);

	printf("\nEnter 2 no:");
	scanf_s("%d", &b);

	result = a + b;
	printf("Addition = %d\n\n", result);

	result = a - b;
	printf("Subtraction = %d\n\n", result);

	result = a * b;
	printf("Multiplication = %d\n\n", result);

	result = a / b;
	printf("Division = %d\n\n", result);

	return 0;


}