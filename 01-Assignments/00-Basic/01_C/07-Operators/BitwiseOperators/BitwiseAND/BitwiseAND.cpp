#include<stdio.h>

int main()
{
	//fuction prototype
	void PrintBinaryFormOFNumber(unsigned int);

	//variable declarations
	unsigned int a, b, result;

	printf("\nEnter no 1:");
	scanf_s("%d", &a);

	printf("\nEnter no 2:");
	scanf_s("%d", &b);


	result = a & b;
	printf("a & b = %d\n\n", result);

	PrintBinaryFormOFNumber(a);
	PrintBinaryFormOFNumber(b);
	PrintBinaryFormOFNumber(result);

	return 0;
}

void PrintBinaryFormOFNumber(unsigned int no)
{
	unsigned int quotient, reminder, num, binary_array[8];
	int i;

	for(i = 0;i < 8;i++)
	{
		binary_array[i] = 0;
	}

	printf("Decimal no = %d\n\n", no);

	num = no;
	i = 7;
	while (num != 0)
	{
		quotient = num / 2;
		reminder = num % 2;
		binary_array[i] = reminder;
		num = quotient;
		i--;
	}
	for (i = 0;i < 10;i++)
	{
		printf("%u", binary_array[i]);
	}
	printf("\n\n");
}