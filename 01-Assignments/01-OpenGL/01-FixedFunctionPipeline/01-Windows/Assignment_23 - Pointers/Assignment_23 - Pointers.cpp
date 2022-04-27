#include<stdio.h>
#include<malloc.h>   //for memory allocation

//for 1D array
void OneDimensional()
{
	int *p;
	int col, i;

	printf("Enter the number of elements:\t");
	scanf_s("%d", &col);

	//memory allocation
	p = (int *)malloc(sizeof(int)* col);

	//take input from user
	printf("Enter the elements:\n");
	for (i = 0;i<col;i++)
	{
		scanf_s("%d", &p[i]);
	}

	//print the output
	printf("Elements in 1D array are\n");
	for (i = 0;i<col;i++)
	{
		printf("Element No %d =\t", i + 1);
		printf("%d\t: Address =\t %p\n", p[i],&p[i]);
		//printf("%p\t", &p[i]);
	}

	printf("\nFreeing the allocated memory for the 1D array\n");
	free(p);
}

//for 2D array
void TwoDimensional()
{
	int **p = NULL;
	int col, row, i, j;

	printf("Enter the number of rows\n");
	scanf_s("%d", &row);

	printf("Enter the number of columns\n");
	scanf_s("%d", &col);

	p = (int **)malloc(sizeof(int)* row);

	for (i = 0;i<row;i++)
	{
		p[i] = (int *)malloc(sizeof(int)*col);
	}

	printf("Enter the elements in the array\n");
	for (i = 0;i<row;i++)
	{
		for (j = 0;j<col;j++)
		{
			scanf_s("%d", &p[i][j]);
		}
	}

	printf("Elements in 2D array are\n");
	for (i = 0;i<row;i++)
	{
		printf("\n");
		for (j = 0;j<col;j++)
		{
			printf("%d\t", p[i][j]);
		}
	}

	printf("\n\n\nAddresses of elements in 2D array are\n");
	for (i = 0;i<row;i++)
	{
		printf("\n");
		for (j = 0;j<col;j++)
		{
			printf("%d\t: Address =\t %p\n", p[i][j], &p[i][j]);
		}
	}


	printf("\nFreeing the allocated memory for the 2D array\n");
	for (i = row-1;i>=0;i--)               //conventionally deallocate memory in reverse order
	{
		free(p[i]);
	}
	free(p);
}

//for 3D array
void ThreeDimensional()
{
	int ***p;
	int first, second, third, i, j, k;

	printf("Enter first dimension\n");
	scanf_s("%d", &first);

	printf("Enter second dimension\n");
	scanf_s("%d",  &second);

	printf("Enter third dimension\n");
	scanf_s("%d", &third);

	p = (int ***)malloc(sizeof(int **)* first);

	for (i = 0;i<first;i++)
	{
		p[i] = (int **)malloc(sizeof(int *)*second);

		for (j = 0;j<second;j++)
		{
			p[i][j] = (int *)malloc(sizeof(int)*third);
		}

	}

	printf("Enter the elements\n");
	for (i = 0;i<first;i++)
	{
		for (j = 0;j<second;j++)
		{
			for (k = 0;k<third;k++)
			{
				scanf_s("%d", &p[i][j][k]);
			}
		}
	}

	//print elements
	printf("Elements in 3D array are\n");
	for (i = 0;i<first;i++)
	{
		for (j = 0;j<second;j++)
		{
			printf("\n");
			for (k = 0;k<third;k++)
			{
				printf("%d", p[i][j][k]);
			}
		}
	}

	//print corresponding address
	printf("\n\n\nAddresses of elements in 3D array are\n");
	for (i = 0;i<first;i++)
	{
		for (j = 0;j<second;j++)
		{
			printf("\n");
			for (k = 0;k<third;k++)
			{
				printf("%d\t: Address =\t %p\n", p[i][j][k], &p[i][j][k]);
			}
		}
	}

	//free memory in loop
	printf("\nFreeing the allocated memory for the 3D array\n");
	for (i = first-1;i>=0;i--)
	{
		for (j = second-1;j>=0;j--)                        //free the memory in opposite in with it is allocated
		{
			free(p[i][j]);
		}
		free(p[i]);
	}
	free(p);
}

int main()
{
	int choice = 0, i = 0;

	printf("1: One Dimensional\n2: Two Dimensional\n3: Three Dimensional\n");
	printf("Enter your choice:\t");
	scanf_s("%d", &choice);

	switch (choice)
	{
	case 1:
		OneDimensional();
		break;

	case 2:
		TwoDimensional();
		break;

	case 3:
		ThreeDimensional();
		break;

	default:
		printf("\nNot a valid choice\n");
	}
	return 0;
}