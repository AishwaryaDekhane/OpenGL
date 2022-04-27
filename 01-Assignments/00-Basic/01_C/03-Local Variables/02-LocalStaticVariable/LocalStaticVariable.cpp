#include<stdio.h>

int main()
{
	int a = 5;

	void cnt();

	printf("\nA=%d\n\n", a);
	cnt();
	cnt();
	cnt();
	return 0;
}

void cnt()
{
	static int l = 0;
	l = l + 1;
	printf("Local count=%d\n", l);
}
