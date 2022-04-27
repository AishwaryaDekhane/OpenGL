#include<stdio.h>

int main()
{
	//code

	printf("next line\nhello\n\n");
	printf("tab\t\ttab\n");
	printf("\'Single quote\'using \\\' \\\'escape\n\n");
	printf("\"double quote\"using \\\" \\\"escape\n\n");
	printf("\'Single quote\'using \\\' \\\'escape\n\n");
	printf("backspace\bback\b hello\\b\n\n");

	printf("\r carriage return\\rescape\n");
	printf("\r carriage \r return\\rescape\n");

	printf("hello \x41 using \\xhh rescape\n\n\n");

	printf("hello \102 using \\ooo rescape\n\n");
}