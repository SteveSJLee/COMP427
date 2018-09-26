// tiny shell
#include <stdlib.h>
#include <stdio.h>

//  tiny shell with built in system function
void system_shell() 
{
	char *line = NULL;
	size_t n = 0;
	ssize_t len = 0;
	while (1) {
		len = getline(&line, &n, stdin);
		if (len > 1)
			system(line);
		else
			exit(0);
	}
}



int main(int argc, char *argv[]) 
{
	printf("Tiny shell implementation using Linux system")
	system_shell();

	return 0;
}