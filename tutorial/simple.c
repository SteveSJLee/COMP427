#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>


double gettime()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME,  &ts) != 0) {
		perror("clock-gettime");
	}
	
	double t = ts.tv_sec *10E9 + ts.tv_nsec;

	return t;
}


int main(int argc, char *argv[]) 
{

	double t1 = gettime();

	fork();

	double t2 = gettime();

	double t_time = t2 - t1;

	printf("total time %fns\n\r", t_time);
	return 0;
}



