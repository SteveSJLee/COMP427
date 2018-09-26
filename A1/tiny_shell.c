// tiny shell
#include <stdlib.h>
#include <stdio.h>

/* funtion prototypes*/
void tshell(void);
int my_system(char* line);
int clone_function(void* arg);





int my_system(char* line)
{
	#ifdef FORK
	// version F

	#elif VFORK
	// version V

	#elif CLONE
	// version C

	#elif PIPE
	// version P

	#else 
		// default will use built in system fn
		system(line);
	#endif

}


//  tiny shell with built in system function
void tshell() 
{
	char *line = NULL;
	size_t n = 0;
	ssize_t len = 0;
	while (1) {
		len = getline(&line, &n, stdin);
		if (len > 1)
			my_system(line);
		else
			exit(0);
	}
}


// main
int main(int argc, char *argv[]) 
{
	printf("Tiny shell implementation using Linux system")
	tshell();

	return 0;
}