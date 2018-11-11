#include <stdio.h>

int main()
{
	char*p = "Hello wold";
	//p[2]= 'C';	//Core dump !!

	int i = 0;
	while(1)
	{
		//if(i<100)
		printf("Hello(%d)\n", i++);
		sleep(1);
		
		if(i>10) break;
	}
	return 0;
}
