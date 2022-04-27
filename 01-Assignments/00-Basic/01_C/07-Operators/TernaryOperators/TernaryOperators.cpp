#include<stdio.h>

int main()
{
	//variable declartion

	int a,b;
	int p,q;
	char chresult1, chresult2;
	int iresult1, iresult2;

	//code

	a = 10;
	b = 20;
	chresult1 = (a > b) ? 'A' : 'B';
	iresult1 = (a > b) ? a : b;
	printf("Ternary --- %c & %d \n\n", chresult1, iresult1);

	p = 30;
	q = 20;
	chresult2 = (p != q) ? 'P' : 'Q';
	iresult2 = (p != q) ? p : q;
	printf("Ternary --- %c & %d \n\n", chresult2, iresult2);

	return 0;


}