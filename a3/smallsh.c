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

#include <stdbool.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

// C
void change_directory(char * dir) {
	if(!chdir(dir) == 0) {
		// chdir was unsuccesful
		perror("cd");
		//printf("changed directory to %s\n", dir);
	}
}

void exit_shell() {
	// printf("Exiting...\n");
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

	// used to store the exit status of the last foreground process
	int fg_exit_status;
	int bg_exit_status;
	int bg_pid;

	while(repeat == 1) {
		// First check to see if any bg processes have finished
		// Cite: Slide 21 lecture 9, and the manpage for wait()
		// TODO: modify this little block to be a funcion that takes a list of bg processes
		// and iterates through those id's so that we don't print foreground process?
		bg_pid = -1;
		bg_pid = waitpid(-1, &bg_exit_status, WNOHANG);
		while( bg_pid != -1 && bg_pid != 0) {
			if(WIFEXITED(bg_exit_status)) {
				printf("pid %d exited normally. Exit status: %d\n", bg_pid, WEXITSTATUS(bg_exit_status));
			}
			else if (WIFSIGNALED(bg_exit_status)) {
				printf("pid %d terminated by signal: %d\n", bg_pid, WTERMSIG(bg_exit_status));
			}
			// printf("pid %d exited. Exit status: %d\n", bg_pid, bg_exit_status);
			bg_pid = waitpid(-1, &bg_exit_status, WNOHANG);
		}

		// Read: Prompt user, get input, and null terminate their string
		printf(": ");
		fflush(stdout);

		fgets(input, sizeof(input), stdin);
		if (strlen(input) > 0)
			input[strlen(input)-1] = '\0'; // removes the newline and replaces it with null

		// When a bg process terminates, a message showing the process id and exit status 
		// will be printed. You should check to see if any background processes completed
		// just before you prompt for a new command and print the message then.
		if (0 == 1 /* any background process completed */) {
			printf("Some background process completed.\n");
			// Use waitpid to check for any completed background processes
			printf("PID %d terminated. Exit status: %d\n", 0, 0);
		}

		// Variables used in parsing input	
		int arg_count = 0, 
			word_count = 0;
		char ** arguments = malloc(MAX_ARGS * sizeof(char *));
		char * words[MAX_ARGS + 1];
		char * command;
		char * input_file;
		char * output_file;
		const char * devnull = "/dev/null/";

		// Booleans to track redirection mode and background vs foreground
		bool bg_mode = false;
		bool redir_input = false;
		bool redir_output = false;

		// Ignore lines that start with # as comments
		if (input[0] == '#') {
			continue;
		}
		
		// Check that the input was not null before evaluating further
		if (command = strtok(input, " ")) {
			// Set the 
			arguments[arg_count++] = command;

			// Tokenize the line, storing words until all arguments are read
			// We are allowed to assume that the command is entered without syntax errors
			while(words[word_count] = strtok(NULL, " ")) {
				char * word = words[word_count]; // Vastly improves readability in this block
				
				// If the current 'word' is begins with '#', change it to null and stop reading additional arguments
				if (word[0] == '#') {
					word[0] = '\0';
					break;
				}

				// If the input redirection operator is present at this word, set input_redirect to 1 (true) and
				// grab the filename (which should be the next word)
				if (strcmp(word, "<") == 0) {
					words[++word_count] = strtok(NULL, " ");
					input_file = words[word_count];
					redir_input = true;
				}
				
				// Likewise check if the output redireect operator was found at this word
				else if (strcmp(word, ">") == 0) {
					words[++word_count] = strtok(NULL, " ");
					output_file = words[word_count];
					redir_output = true;
				}

				// Set BG mode if word is '&'
				else if (strcmp(word, "&") == 0) {
					bg_mode = true;
					// Make sure there's nothing else after the &
					if (words[++word_count] = strtok(NULL, " ")) {
						printf("Usage: '&' must be last word of the command\n");
						break;
					}
				}

				// Everything else should be an argument 
				// We are assuming user uses correct syntax, especially for redirection syntax
				else {
					arguments[arg_count++] = words[word_count++];
				}
			}

			// Null terminate the arguments array right after last argument added
			// Failure to do this can cause any call after the first to cause some serious
			// bugs, like exec() calls trying to send arguments when none are entered after
			// we have entered any command with multiple arguments. This fixes that bug.
			// Note that execvp() and execlp() reads from arguments until NULL is found.
			arguments[arg_count] = NULL;

			// Built-in command evaluation. Not sure if should be able to redirect - guessing not?
			// exit builtin: exits the smallsh shell
			if (strcmp(command, "exit") == 0) {
				// Free local malloc for arguments before going into our exit_shell process
				free(arguments);
				exit_shell();
			}

			// cd builtin: Change directory. If no argument provided, cd to HOME env. variable
			else if (strcmp(command, "cd") == 0) {
				// If only argument is the 'cd' command, change to HOME path variable directory
				if (arg_count == 1) {
					// Use getenv to lookup an environment var
					// Cite: TLPI Pg 127. 
					change_directory(getenv("HOME"));
				}

				// else if argument was provided to cd, attempt to change to that directory
				else if (arg_count == 2) {
					change_directory(arguments[1]);
				}

				// else more arguments were provided, print usage message.
				else {
					printf("smallsh: cd: usage: cd [directory]\n");
				}
			}

			// status builtin: Provides exit status of last foreground command that exited
			else if (strcmp(command, "status") == 0) {
				// Send the exit status to the current output (stdout or file)
				printf("exit value %d\n", fg_exit_status);

				

			}

			else {
				// If input file was given, open the input file for reading only and us it as input

				// If an output file was given, open the output file for reading only and use as output

				// If process was set to run as a bg process and no output file was given
				// then set the output of the bg process to dev/null


				// Run the command using exec  / fork family of functions as appropriate
				// Cite: Slides from Lecture 9, especially, 28
				// Cite: brennan.io/2015/01/16/write-a-shell-in-c/  (for launching process and waiting until it's done)
				pid_t pid = fork(); // Parent process gets pid of child assigned, child gets 0
				pid_t wait_pid;

				// Child process has pid 0 if fork() success
				if (pid == 0) {	
					// Attempt to execute the command, print error if fails
					if(execvp(command, arguments)  == -1) {
						perror("smallsh");
					}
					exit(EXIT_FAILURE); // Only executes if execvp() failed
				}
				else if (pid == -1) {
					// Fork() failed to create child, report the error
					perror("fork()");
				}
				else {
					// Parent process will execute this code:
					if (bg_mode) {
						printf("TODO: exec in bg mode\n");
						// Execute in background mode and print pid of bg process
					}
					else {
						do {
							wait_pid = waitpid(pid, &fg_exit_status, 0); // WUNTRACED);
						} while (!WIFEXITED(fg_exit_status) && !WIFSIGNALED(fg_exit_status));
						// Todo: How to print exit status of process properly??
						if(fg_exit_status != 0) {
							printf("%s: terminated by signal %d\n", command, fg_exit_status);
						}
					}
				}
			}
		}

		free(arguments);
	}
}

// signal handler for SIGINT that does NOT terminate this shell,
// but instead terminates the foreground process
// Cite: TLPI Page 399
static void sigHandler(int sig) {
	if (sig == SIGINT) {
		printf("SIGINT caught\n");
		// Figure out how to terminate just the foreground child process and do that here
		// Get foreground process and terminate it (send  SIGINT I guess?)
		// Cite: TLPI Section 20.5
		pid_t fg_process = 0; // TODO: Get the actual foreground process id
		int signalb = 0;
		kill(fg_process, signalb); // Send SIGINT signal to foreground process instead
		
		// If a command (BG or FG) is terminated by a signal, message indicated which
		// signal terminated the process will be printed
		printf("Signal: %d\n", sig);
		command_prompt();
	}
	else {
		printf("Signal: %d", sig);
	}
}


// Used for debugging purposes
void print_array (char ** arguments, int n) {
	int i = 0;
	for (i = 0; i < n; i++) {
		printf("arg%d: %s\n", i, arguments[i]);
	}
}


// main
int main(int argc, char const *argv[])
{



	command_prompt();
	return 0;
}



// DEBUG //////////////////////////////////////////////
			// printf("command: %s\n", command);
			// print_array(arguments, arg_count);
			// printf("arg_count: %d\n", arg_count);
			// printf("Input redirection: %d\n", redir_input);
			// printf("output_file: %s\n", output_file);
			// printf("Output redirection: %d\n", redir_output);
			// printf("input_file: %s\n", input_file);
			// printf("bg_mode: %d\n", bg_mode);
// DEBUG //////////////////////////////////////////////