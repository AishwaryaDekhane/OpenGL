#include<stdio.h>

// GLOBAL SCOPE

int main()
{
	//fuction prototype

	
	void change_count(void);
	
	extern int global_count;

	//code
	printf("\n");
	printf("Global count= %d\n", global_count);
	change_count();
	
	return 0;
}
int global_count = 0;

//** GLOBAL SCOPE ** 
void change_count(void)
{
	global_count = global_count + 1;
	printf("Global count =%d\n", global_count);
}


