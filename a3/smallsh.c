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

/* Externs */
extern char **environ; // pointer to strings listing the environment variables

void change_directory(char * dir) {
	// If empty string passed in, update to the HOME path
	if (strcmp(dir, "") == 0 || dir == NULL) {
		// Cite: TLPI Pg 127. Using getenv to lookup an environment var:
		dir = getenv("HOME");
	}
	printf("Changing directory to:  %s\n", dir);

	if(chdir(dir) == 0) {
		// chdir was succesful
		printf("changed directory to %s\n", dir);
	} else {
		printf("error changing to %s\n", dir);
		// printf("error number: %s\n", errno);
	}

}

void exit_shell() {
	printf("Exiting...\n");
	// Kill all child processes / jobs
	// terminate smallsh itself by calling exit with success signal (0)
	exit(0);
}

void command_prompt() {
	// Start up signal handler
	// Cite: TLPI Chapter 20, ~Page 401 and other examples
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
		int arg_count = 0;
		char * arguments[MAX_ARGS];
		char * command;

		// Ignore lines that start with # as comments
		if (input[0] == '#') {
			continue;
		}

		command = strtok(input, " ");
		while(arguments[arg_count] = strtok(NULL, " ")) {
			printf("arguments[%d]: %s\n", arg_count, arguments[arg_count]);
			// Add logic here to parse for arguments -- check if arguments[arg_count][0] == '-'
			arg_count++;
		}
		printf("command = %s\n", command);

		// Check for the built-in commands:
		if (strcmp(command, "exit") == 0) {
			exit_shell();
		}

		// If no-arguments 'cd' command
		else if (strcmp(command, "cd") == 0) {
			if (arg_count == 0)
				change_directory("");
			else
				change_directory(arguments[0]);
		}

		// If cd with a directory passed
		// else if (str)



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