/*******************************************************************************
* File:         smallsh.c
* Author:       Shawn S Hillyer
* Date:         , 2016
* Course:       OSU CSS 344-400: Assignment 03
* Description:  
* 
* 
* 
*******************************************************************************/

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Constants */
#define MAX_CHAR 2048
#define MAX_ARGS 512


/* Forward declarations */
void command_prompt();
void 
exit_shell() {
	printf("Exiting...\n");
	// Kill all child processes / jobs
	// terminate smallsh itself by calling exit with success signal (0)
	exit(0);
}

void 
command_prompt() {
	// support MAX_CHAR characters + 1 for null byte
	char input[MAX_CHAR+1]; 
	int repeat = 1;

	while(repeat == 1) {
		// Read: Prompt user, get input, and null terminate their string
		printf(": ");
		fgets(input, sizeof(input), stdin);
		input[strlen(input)-1] = '\0';

		// DEBUG: Echo the input
		printf("You input: %s\n", input);

		/* Parse the input  */
		
		int arg_count;
		

		if (strcmp(input, "exit") == 0) {
			exit_shell();
		}
	}

}

// Need to build a signal handler for SIGINT that does NOT terminate this process
// but instead terminates the foreground process

int 
main(int argc, char const *argv[])
{
	command_prompt();
	return 0;
}