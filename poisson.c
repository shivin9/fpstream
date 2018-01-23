#include<stdio.h>
#include<stdlib.h>
#include<math.h>

double nextTime(double rateParameter)
{
	return -logf(1.0f - (double) random() / (RAND_MAX)) / rateParameter;
}
int main()
{
	srand(time(NULL));
	int i;
	for(i = 0; i < 10000000; i++)
	{
		printf("%lf\n", nextTime(1));
	}
return 0;
}
