#include<stdio.h>

#define my_pi 3.132

enum
{
	Jan = 1,
	Feb,
	Mar,
	Apr,
	May,
	Jun,
	Jul,
	Aug,
	Sept,
	Oct,
	Nov,
	Dec
};
enum
{
	Sun,
	Mon,
	Tues,
	Wed,
	Thur,
	Fri,
	Sat
};
enum boolean
{
	TRUE=1,
	FALSE=0
};
int main()
{
	//local constants declarations

	const double epsilon = 0.000001;

	//code
	printf("\nlocal constant epsilon =%f \n\n", epsilon);

	printf("Sun is day no = %d \n", Sun);
	printf("Mon is day no = %d \n", Mon);
	printf("Tues is day no = %d \n", Tues);
	printf("Wed is day no = %d \n", Wed);
	printf("Thurs is day no = %d \n", Thur);
	printf("Fri is day no = %d \n", Fri);
	printf("Sst is day no = %d \n", Sat);

	printf("Jan month no is = %d \n", Jan);
	printf("Feb month no is = %d \n", Feb);
	printf("Mar month no is = %d \n", Mar);
	printf("Apr month no is = %d \n", Apr);
	printf("May month no is = %d \n", May);
	printf("Jun month no is = %d \n", Jun);
	printf("Jul month no is = %d \n", Jul);
	printf("Aug month no is = %d \n", Aug);
	printf("Sept month no is = %d \n", Sept);
	printf("Oct month no is = %d \n", Oct);
	printf("Nov month no is = %d \n", Nov);
	printf("Dec month no is = %d \n", Dec);

	printf("Value of TRUE is = %d \n", TRUE);
	printf("Value of FALSE is = %d \n", FALSE);

	printf("My_pi value is = %10lf\n\n", my_pi);

	return 0;
}