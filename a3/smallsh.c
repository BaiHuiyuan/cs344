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
static void sigHandler(int sig);


void change_directory(char * dir) {
	// If empty string passed in, go to the HOME path
	if (strcmp(dir, "") == 0) {
		dir = "HOME path directory";
	}
	printf("Changing directory to:  %s\n", dir);

	chdir(dir);

}

void exit_shell() {
	printf("Exiting...\n");
	// Kill all child processes / jobs
	// terminate smallsh itself by calling exit with success signal (0)
	exit(0);
}

void command_prompt() {
	// Start up signal handler
	if (signal(SIGINT, sigHandler) == SIG_ERR) {
		printf("SIG_ERR received\n");
	}

	// support MAX_CHAR characters + 1 for null byte
	char input[MAX_CHAR+1]; 
	int repeat = 1;

	while(repeat == 1) {
		// Read: Prompt user, get input, and null terminate their string
		printf(": ");
		fgets(input, sizeof(input), stdin);
		input[strlen(input)-1] = '\0';

		// DEBUG: Echo the input
		printf("input: %s\n", input);

		/* Parse the input  */
		
		int arg_count;

		// Check for the built-in commands:
		if (strcmp(input, "exit") == 0) {
			exit_shell();
		}
		else if (strcmp(input, "cd") == 0) {
			change_directory("");
		}


		// Ignore lines that start with # as comments
		else if (input[0] == '#') {
			continue;
		}

		// Else if only whitespace or a newline entered then just reprompt
		else {
			continue;
		}
	}

}

// Need to build a signal handler for SIGINT that does NOT terminate this process
// but instead terminates the foreground process
// Cite: tLPI Page 399

static void sigHandler(int sig) {
	if (sig == SIGINT) {
		printf("SIGINT caught\n");
		// Figure out how to terminate just the foreground child process and do that here
	}
	return;
}


int main(int argc, char const *argv[])
{

	command_prompt();
	return 0;
}