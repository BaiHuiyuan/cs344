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
void print_array (char ** arguments, int size);

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
		if (strlen(input) > 0)
			input[strlen(input)-1] = '\0'; // removes the newline and replaces it with null

		// DEBUG: Echo the input
		printf("input: %s\n", input);

		// Variables used in parsing input	
		int arg_count = 0, 
			word_count = 0;
		char * arguments[MAX_ARGS];
		char * words[MAX_ARGS + 1];
		char * command;
		char * input_file;
		char * output_file;
		const char * devnull = "/dev/null/";

		// Booleans to track redirection mode and background vs foreground
		int bg_mode = 0;
		int redir_input = 0;
		int redir_output = 0;

		// Ignore lines that start with # as comments
		if (input[0] == '#') {
			continue;
		}
		
		// Check that the input was not null before evaluating further
		if (command = strtok(input, " ")) {
			// Tokenize the line, storing words until all arguments are read
			// We are allowed to assume that the command is entered without syntax errors
			while(words[word_count] = strtok(NULL, " ")) {
				char * word = words[word_count]; // Vastly improves readability in this block
				
				// If the current 'word' is begins with '#', change it to null and stop reading additional arguments
				if (word[0] == '#') {
					word[0] = '\0';
					break;
				}

				// If the input redirection operator is present, set input_redirect to 1 (true) and
				// grab the filename (which should be the next word)
				else if (strcmp(word, "<") == 0) {
					words[++word_count] = strtok(NULL, " ");
					input_file = words[word_count];
					redir_input = 1;
				}
				
				// Likewise check if the output redireect operator was found
				else if (strcmp(word, ">") == 0) {
					words[++word_count] = strtok(NULL, " ");
					output_file = words[word_count];
					redir_output = 1;
				}

				// Set BG mode if word is '&'
				else if (strcmp(word, "&") == 0) {
					bg_mode = 1;
					// Make sure there's nothing else after the &
					if (words[++word_count] = strtok(NULL, " ")) {
						perror("Usage error: '&' must be last word of the command\n");
						break;
					}
				}

				// Everything else should be an argument assuming user uses correct syntax
				else {
					arguments[arg_count++] = words[word_count++];
				}
			}


			// Set the background mode to true if the last 'argument' is '&'
			if (arg_count > 1) {
				if (strcmp(arguments[arg_count - 1], "&") == 0) {
					printf("Background process.\n");
					// bg_mode = 1;
				}
			}


			printf("command: %s\n", command);
			print_array(arguments, arg_count);
			printf("arg_count: %d\n", arg_count);
			printf("Input redirection: %d\n", redir_input);
			printf("output_file: %s\n", output_file);
			printf("Output redirection: %d\n", redir_output);
			printf("input_file: %s\n", input_file);
			printf("bg_mode: %d\n", bg_mode);


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

		

}

// signal handler for SIGINT that does NOT terminate this shell,
// but instead terminates the foreground process
// Cite: TLPI Page 399
static void sigHandler(int sig) {
	if (sig == SIGINT) {
		printf("SIGINT caught\n");
		// Figure out how to terminate just the foreground child process and do that here
	}
	return;
}


// Used for debugging purposes
void print_array (char ** arguments, int n) {
	int i = 0;
	while (i < n) {
		printf("arg%d: %s\n", i, arguments[i++]);
	}
}


// main
int main(int argc, char const *argv[])
{



	command_prompt();
	return 0;
}