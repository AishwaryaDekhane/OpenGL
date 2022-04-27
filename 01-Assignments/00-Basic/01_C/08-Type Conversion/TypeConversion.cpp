#include<stdio.h>

int main()
{
	int i, j;
	char ch1, ch2;

	int a, ires;
	float f, fres;

	int iexplicit;
	float fexplicit;

	//code

	i = 68;
	ch1 = i;
	printf("i = 5d\n", i);

	printf("char 1 = %c\n", ch1);

	ch2 = 'Q';
	j = ch2;
	printf("char 2 = %c \n", ch2);
	printf("j (j= char 2) = %d\n", j);

	a = 5;
	f = 3.14f;

	fres = a + f;
	printf("Float result = %f\n\n", fres);

	ires = a + f;
	printf("Int result = %d\n\n", ires);

	fexplicit = 50.3684f;
	iexplicit = (int)fexplicit;

	printf("Float = %f\n", fexplicit);
	printf("Int = %d\n", iexplicit);

	return 0;
}