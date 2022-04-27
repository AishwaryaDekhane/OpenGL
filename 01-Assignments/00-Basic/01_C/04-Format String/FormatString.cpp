#include<stdio.h>
int main(int argc, char *argv[], char *envp[])
{
	printf("\n Hello Aishwarya \n");
	
	int AMD_a = 13;

	printf("Integer decimal value of 'A' = %d\n", AMD_a);
	printf("Integer ocatal value of 'A' = %o\n", AMD_a);
	printf("Integer hexadecimal value of 'A' = %x\n", AMD_a);
	printf("Integer hexadecimal value of 'A' = %X\n", AMD_a);


	char ch = 'P';
	printf("character ch = %c\n", ch);
	char str[] = "Aishwarya";
	printf("String str = %s\n", str);

	long num = 387461L;
	printf("Long Integer = %ld\n", num);

	unsigned int b = 3;
	printf("unsigned int b = %u\n", b);

	float f = 30.91f;
	printf("Floating pt no with %%f f = %f\n", f);
	printf("Floating pt no with %%4.2f f = %4.2f\n", f);
	printf("Floating pt no with %%2.5f f = %2.5f\n", f);

	double d_pi = 3.1487365846987;
	printf("Double Precision floating pt number without exponential = %g\n", d_pi);
	printf("Double Precision floating pt number without exponential = %e\n", d_pi);
	printf("Double Precision floating pt number without exponential = %E\n", d_pi);
	printf("Double hexadecimal value of d_pi(lower case) = %a\n", d_pi);
	printf("Double hexadecimal value of d_pi(upper case) = %A\n", d_pi);

	return 0;
}

