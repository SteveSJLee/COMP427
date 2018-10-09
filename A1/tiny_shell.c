/* defines */
#define _GNU_SOURCE			// clone()
#define WRITE_PIPE 1
#define READ_PIPE 0
#define STACK_SIZE 8192
//#define PRINT_TIME

/* include headers */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>			// execl(), pipe()
#include <sys/wait.h>		// waitpid()
#include <time.h>			// clock_gettime()
#include <string.h>			// strcmp()
#include <sched.h>			// clone()
#include <sys/types.h>		// open()
#include <sys/stat.h>		// open()
#include <fcntl.h>			// open()
#include <errno.h>			// for errors

/* funtion prototypes */
int checkExit(char* s);							// check if string is 'exit'
double getTime(void);							// get current time when it is called
void printTime(double t_start, double t_end);	// print difference of t1 and t2
void version_F(char* line);						// fork()
void version_V(char* line);						// vfork()
static int childFunc(void *arg);				// child function for clone()
void version_C(char* line);						// clone()
void version_P(char* line);						// pipe implementation
void my_system(char* line);						// function containing various system() implementations
void tshell(void);								// tiny shell 

/* global variables */
double t1, t2;
char* myfifo;
int stream;

/* user function */

/* print the difference of given 2 time t1,t2 */
void printTime(double t_start, double t_end)
{
	double t_total = t_end - t_start;
	#ifdef PRINT_TIME
	printf("Total execution time: %fs\n\r", t_total);
	#endif
}


/* given an input char array s, check first 4 character is exit */
int checkExit(char* s)
{
	int sum = 0;
	if (s[0] == 'e' || s[0] == 'E') {
		sum = sum + 1;
	} 
	if (s[1] == 'x' || s[1] == 'X') {
		sum = sum + 1;
	} 
	if (s[2] == 'i' || s[2] == 'I') {
		sum = sum + 1;
	} 
	if (s[3] == 't' || s[3] == 'T') {
		sum = sum + 1;
	} 
	if (sum == 4) return 1;
	
	return 0;
}


/* get current time using cock_gettime function */
double getTime()
{
	struct timespec ts;
	double t;
	
	if (clock_gettime(CLOCK_REALTIME,  &ts) != 0) {
		perror("clock-gettime");
	}
	// calculate time in nano seconds
	t = ts.tv_sec + ts.tv_nsec/1E9;

	return t;
}


/* my system implementation with fork() */
void version_F(char* line)
{
	int status;
	pid_t pid;
	
	t1 = getTime();		// get start time
	pid = fork();
	t2 = getTime();		// get end time
	if (pid == -1) {
		// error in fork()
		perror("error: fork()\r\n");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		printTime(t1, t2);
		// I'm child process, execute command 
		if (execl("/bin/sh", "sh", "-c", line, (char *)0) == -1) {
			perror("error: execl()\r\n");
			exit(EXIT_FAILURE);
		} else {
			exit(EXIT_SUCCESS);
		}
		
	} else {
		// I'm parent process
		// wait for child process to finish
		if (waitpid(pid, &status, 0) == -1) {
			perror("error: waitpid()\r\n");
			exit(EXIT_FAILURE);
		}
	}
}


/* my system implementation with vfork() */
void version_V(char* line)
{
	int status;
	pid_t pid;
	
	t1 = getTime();		// get start time
	pid = vfork();
	t2 = getTime();		// get end time
	if (pid == -1) {
		// error in fork()
		perror("error: vfork()\r\n");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		printTime(t1, t2);
		// I'm child process, execute command 
		if (execl("/bin/sh", "sh", "-c", line, (char *)0) == -1) {
			perror("error: execl()\r\n");
			exit(EXIT_FAILURE);
		} else {
			exit(EXIT_SUCCESS);
		}		
	} else {
		// I'm parent process
		// wait for child process to finish
		if (waitpid(pid, &status, 0) == -1) {
			perror("error: waitpid()\r\n");
			exit(EXIT_FAILURE);
		}
	}
}


/* child function for clone() */
static int childFunc(void *arg) 
{
	if (execl("/bin/sh", "sh", "-c", arg, (char *)0) == -1) 
	{
		perror("error: execl()\r\n");
		exit(EXIT_FAILURE);
	} 
	return 0;
}


/* system implementation with clone() */
void version_C(char* line)
{
	char *stack;                    
	char *stackTop;                 
	pid_t pid;
    
	stack = malloc(STACK_SIZE);
    if (stack == NULL) 
	{
		perror("error: malloc()\r\n");
		exit(EXIT_FAILURE);
	}
    stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

	/* Flags used and descriptions
		CLONE_FS	: Parent and child processes will share same filesystem information.
		SIGCHLD		: send signal when a child process terminates
	*/
	t1 = getTime();		// get start time
	pid = clone(childFunc, stackTop, CLONE_FS | SIGCHLD, line);
	t2 = getTime();		// get end time
	printTime(t1, t2);
	if (pid == -1) 
	{
		perror("error: clone()\r\n");
		exit(EXIT_FAILURE);
	}
	if (waitpid(pid, NULL, __WALL) == -1) 
	{
		perror("error: waitpid()\r\n");
		exit(EXIT_FAILURE);
	}
}


/* implementation of PIPE using fork() */
void version_P(char* line)
{
	int status, fd;
	int pipefd[2] = {0,0};
	pid_t pid;
	
	// create a fifo with read/write permission
	while (mkfifo(myfifo, 0666) == -1) { 
		if (errno == EEXIST) {
			// if pipe is already created or fifo with same name already exists,
			// ignore the error 
			break;
		} else {
			perror("error: mkfifo()\r\n");
			exit(EXIT_FAILURE);
		}
	}
	pipe(pipefd);	// create a pipe
	
	pid = fork();
	if (pid == -1) {
		// error in fork()
		perror("error: fork()\r\n");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		if (stream == WRITE_PIPE) {
			close(stream);
			// open fifo in write only mode
			fd = open(myfifo, O_WRONLY);
			if (fd == -1)
				perror("error: open()\r\n");
			
			// I'm child process, execute command 
			if (execl("/bin/sh", "sh", "-c", line, (char *)0) == -1) {
				perror("error: execl()\r\n");
				exit(EXIT_FAILURE);
			} else {
				exit(EXIT_SUCCESS);
			}
			close(fd);
		} else if (stream == READ_PIPE) {
			close(stream);
			// open fifo in read only mode
			fd = open(myfifo, O_RDONLY);
			if (fd == -1)
				perror("error: open()\r\n");
			
			// I'm child process, execute command 
			if (execl("/bin/sh", "sh", "-c", line, (char *)0) == -1) {
				perror("error: execl()\r\n");
				exit(EXIT_FAILURE);
			} else {
				exit(EXIT_SUCCESS);
			}
			close(fd);
		}
	} else {
		// I'm parent process
		// wait for child process to finish
		if (waitpid(pid, &status, 0) == -1) {
			perror("error: waitpid()\r\n");
			exit(EXIT_FAILURE);
		}
		// remove my fifo
		unlink(myfifo);
	}
}


/* choose my system based on define */
void my_system(char* line)
{
	//t1 = getTime();		// get start time
	#ifdef FORK
	// version F
		version_F(line);
	#elif VFORK
	// version V
		version_V(line);
	#elif CLONE
	// version C
		version_C(line);
	#elif PIPE
	// version P
		version_P(line);
	#else 
		// default will use built in system fn
		if (system(line) == -1) 
		{
			perror("error: system()\r\n");
			exit(EXIT_FAILURE);
		}
	#endif
	//t2 = getTime();		// get end time
	//printTime(t1, t2);
}


/* simple shell with built with my_system() */
void tshell() 
{
	char *line = NULL;
	size_t n = 0;
	ssize_t len = 0;
	
	while (1) {
		printf("[tshell~] ");
		len = getline(&line, &n, stdin);
		if (len > 0) {
			// if input is exit, exit the program
			if (checkExit(line) == 1 && len == 5) {
				printf("\nGoodbye\r\n");
				break;
			// ignore if user entered just 'enter' key
			} else if (line[0] != '\n') {
				if (len != 1) my_system(line);		
			}
		}
		else {
			exit(0);
		}
	}
	free(line);
}


/* main */
int main(int argc, char *argv[]) 
{
	printf("-------------------------------------------------------\r\n");
	printf("[ECSE-427 Assignment 1]\r\n");
	printf("Tiny shell implementation using:\n\tfork(), vfork() and clone()\r\n");
	printf("Written by Steve Lee\r\n");
	printf("-------------------------------------------------------\r\n\n");
	
	#ifdef PIPE
	if (argc == 3) {
		myfifo = argv[1];
		if (*(argv[2]) == '0') 
			stream = 0;
		else if (*(argv[2]) == '1')
			stream = 1;	
	}
	#endif
	
	// run tiny shell
	tshell();

	return 0;
}
