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

	printf("0 = FALSE\n");
	printf("1 = TRUE\n");

	result = (a < b);
	printf("(a < b) = %d\n\n", result);

	result = (a > b);
	printf("(a > b) = %d\n\n", result);

	result = (a <= b);
	printf("(a <= b) = %d\n\n", result);

	result = (a >= b);
	printf("(a >= b) = %d\n\n", result);

	result = (a == b);
	printf("(a == b) = %d\n\n", result);

	result = (a != b);
	printf("(a != b) = %d\n\n", result);

	return 0;


}