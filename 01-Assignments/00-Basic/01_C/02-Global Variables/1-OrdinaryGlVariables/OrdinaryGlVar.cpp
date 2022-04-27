#include<stdio.h>

/*GLOBAL SCOPE

--> If not initialized by us then it will be initialized to zero
*/

int i = 0;

int main()
{
	//function prototypes

	void change_count_one(void);
	void change_count_two(void);

	//code
	printf("\n");
	printf("Global count= %d\n", i);
	change_count_one();
	change_count_two();
	return 0;
}

//** GLOBAL SCOPE ** 
void change_count_one(void)
{
	i = i + 1;
	printf("Global count =%d\n", i);
}

void change_count_two(void)
{
	i = i + 1;
	printf("Global count =%d\n", i);
}
