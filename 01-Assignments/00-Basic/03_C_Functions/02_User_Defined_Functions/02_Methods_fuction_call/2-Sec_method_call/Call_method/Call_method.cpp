#include<stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	void display(void);
	void fun_country(void);

	display();
	fun_country();

	return 0;
}

void display(void)
{
	void fun_my(void);
	void fun_name(void);
	void fun_is(void);
	void fun_fn(void);
	void fun_mn(void);
	void fun_s(void);
	void fun_ogl(void);


	fun_my();
	fun_name();
	fun_is();
	fun_fn();
	fun_mn();
	fun_s();
	fun_ogl();
}

void fun_my(void)
{
	printf("\nMy\n");
}

void fun_name(void)
{
	printf("\nName\n");
}

void fun_is(void)
{
	printf("\nis\n");
}

void fun_fn(void)
{
	printf("\nAishwarya\n");
}

void fun_mn(void)
{
	printf("\nMangesh\n");
}

void fun_s(void)
{
	printf("\nDekhane\n");
}

void fun_ogl(void)
{
	printf("\nlearning OpenGL\n");
}

void fun_country(void)
{
	printf("\nI live in INDIA in Pune\n");
}