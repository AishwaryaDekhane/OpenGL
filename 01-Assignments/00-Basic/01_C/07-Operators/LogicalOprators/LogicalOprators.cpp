#include<stdio.h>

int main()
{
	//variable declartion

	int a;
	int b;
	int c;
	int result;

	//code

	printf("\nEnter no 1:");
	scanf_s("%d", &a);

	printf("\nEnter no 2:");
	scanf_s("%d", &b);

	printf("\nEnter no 3:");
	scanf_s("%d", &c);

	printf("\n0 = FALSE\n");
	printf("1 = TRUE\n\n");

	result = (a <= b) && (b != c);
	printf("(a <= b) && (b != c) = %d\n\n", result);

	result = (b >= a) || (a == c);
	printf("(b >= a) || (a == c) = %d\n\n", result);

	result = !a;
	printf("!a = %d\n\n", result);

	result = !b;
	printf("!b = %d\n\n", result);

	result = !c;
	printf("!c = %d\n\n", result);

	result = (!(a <= b) && !(b != c));
	printf("(!(a <= b) && !(b != c)) = %d\n\n", result);

	result = !((b >= a) || (a == c));
	printf("!((b >= a) || (a == c)) = %d\n\n", result);

	return 0;


}