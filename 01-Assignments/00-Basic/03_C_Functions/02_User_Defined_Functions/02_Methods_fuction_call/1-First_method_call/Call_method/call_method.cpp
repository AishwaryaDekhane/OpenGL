#include<stdio.h>

int main(int argc, char *argv[], char*envp[])
{
	void add(void);
	int sub(void);
	void mul(int, int);
	int div(int, int);

	int sres;
	int no1_mul, no2_mul;
	int no1_div, no2_div, res_div;


	//code

	add();

	sres = sub();
	printf("Sub = %d\n\n", sres);


	printf("Enter no1 for mul");
	scanf_s("%d", &no1_mul);

	printf("Enter no2 for mul");
	scanf_s("%d", &no2_mul);

	mul(no1_mul, no2_mul);



	printf("Enter no1 for div");
	scanf_s("%d", &no1_div);

	printf("Enter no2 for div");
	scanf_s("%d", &no2_div);

	res_div = div(no1_div, no2_div);

	printf("Div of %d & %d = %d\n", no1_div, no2_div, res_div);

	return 0;

}

void add(void)
{
	int no1, no2, isum;

	printf("Enter no1 for add");
	scanf_s("%d", &no1);

	printf("Enter no2 for add");
	scanf_s("%d", &no2);

	isum = no1 + no2;

	printf("Sum of %d & %d = %d\n\n\n", no1, no2, isum);
}

int sub(void)
{
	int no1, no2, isub;

	printf("Enter no1 for sub");
	scanf_s("%d", &no1);

	printf("Enter no2 for sub");
	scanf_s("%d", &no2);

	isub = no1 - no2;

	return (isub);
}

void mul(int no1_mul,int no2_mul)
{
	int imul;

	imul = no1_mul * no2_mul;

	printf("Mul of %d & %d = %d\n", no1_mul, no2_mul, imul);
}

int div(int no1_div,int no2_div)
{
	int idiv;

	if (no1_div > no2_div)
	{
		idiv = no1_div / no2_div;
	}
	else
	{
		idiv = no2_div / no1_div;
	}
	return (idiv);
}