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
static void sig_handler(int sig);
void print_array (char ** arguments, int size);

/* Externs */
extern char **environ; // pointer to strings listing the environment variables

/* Struct for storing dynamic array of process id's running in background */
// Cite: CS 261 - dynamic array materials.
// Decided to do this because I couldn't think of an easy way to keep a dynamic
// list of bg processes to review before each prompt to check if they had ended
// or not. Also, when exiting shell, want to be able to iterate over all bg processes
// and kill them
struct pid_arr {
	pid_t * pids; // array of process id's
	int capacity;
	int size;
};

// Make a single pid_arr in global scope
struct pid_arr bg_pids;

void init_bg_process_arr() {
	bg_pids.capacity = 10;
	bg_pids.size = 0;
	bg_pids.pids = malloc(sizeof(pid_t) * bg_pids.capacity );
}

bool pid_arr_is_full() {
	return (bg_pids.size >= bg_pids.capacity);
}

void push_bg_pid(pid_t pid) {
	// If array is full, reallocate to twice size
	if (pid_arr_is_full()) {
		bg_pids.capacity *= 2;
		bg_pids.pids = realloc(bg_pids.pids, sizeof(pid_t) * bg_pids.capacity );
	}
	// TODO: if time, halve memory if less than 1/4 full
	bg_pids.pids[bg_pids.size++] = pid;
}

bool is_bg_pid(pid_t pid) {
	int i;
	for (i = 0; i < bg_pids.size; i++) {
		if (bg_pids.pids[i] == pid)
			return true;
	}
	return false;
}

void kill_all_bg_pids() {
	// FG process should just handle signal as normal, need to go through bg processes
	// Cite: TLPI Section 20.5
	pid_t bg_process;
	int i;
	for (i = 0; i < bg_pids.size; i++) {
		bg_process = bg_pids.pids[i];
		kill(bg_process, SIGKILL); // kill all child processes
	}
}

void change_directory(char * dir) {
	if(!chdir(dir) == 0) {
		// chdir was unsuccesful
		perror("cd");
		//printf("changed directory to %s\n", dir);
	}
}

void exit_shell() {
	// Kill all child processes / jobs
	kill_all_bg_pids();
	// terminate smallsh itself by calling exit with success signal (0)
	exit(0);
}


void check_bg_processes() {
	// Cite: Slide 21 lecture 9, and the manpage for wait()
	// iterates through all the bg id's so that we don't print foreground process?
	pid_t bg_pid = -1;
	int i;
	int bg_exit_status;

	for (i = 0; i < bg_pids.size; i++) {
		bg_pid = waitpid(bg_pids.pids[i], &bg_exit_status, WNOHANG);
		if (bg_pid != 0 && bg_pid != -1) {
			// If wait exited with a status code, display it:
			if(WIFEXITED(bg_exit_status)) {
				printf("background pid %d is done: exit value %d\n", bg_pid, WEXITSTATUS(bg_exit_status));
			}
			// if it was terminated by signal, display the code:
			else if (WIFSIGNALED(bg_exit_status)) {
				printf("background pid %d is done: terminated by signal %d\n", bg_pid, WTERMSIG(bg_exit_status));
			}
		}
	}
}


void command_prompt() {
	// support MAX_CHAR characters + 1 for null byte
	char input[MAX_CHAR+1]; 
	int repeat = 1;
	int fg_exit_status;
	const char * devnull = "/dev/null";

	while(repeat == 1) {
		// First check to see if any bg processes have finished. If so, keep checking.
		check_bg_processes();

		// Read: Prompt user, get input, and null terminate their string
		printf(": ");
		fgets(input, sizeof(input), stdin);
		fflush(stdout);
		fflush(stdin);
		if (strlen(input) > 0)
			input[strlen(input)-1] = '\0'; // removes the newline and replaces it with null

		// Variables used in parsing input	
		int arg_count = 0, 
			word_count = 0;
		char ** arguments = malloc(MAX_ARGS * sizeof(char *));
		char * words[MAX_ARGS + 1];
		char * command = NULL;
		char * input_file = NULL;
		char * output_file = NULL;
		
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
				
				// all others are arguments (we assume user does correct syntax)
				else {
					arguments[arg_count++] = words[word_count++];
				}
			} // end parsing data block

			/* Null terminate the arguments array right after last argument added
			Failure to do this can cause any call after the first to cause some serious
			bugs, like exec() calls trying to send arguments when none are entered after
			we have entered any command with multiple arguments. This fixes that bug.
			Note that execvp() and execlp() reads from arguments until NULL is found. */
			arguments[arg_count] = NULL;

			/***********************************************************************************
			* Built-in command evaluation. These do NOT have to worry about redirection
			***********************************************************************************/
			
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


			/***********************************************************************************
			* Non builtins are executed by forking and calling exec
			* Cite: Slides from Lecture 9, especially, 28
			* Cite: brennan.io/2015/01/16/write-a-shell-in-c/  (for launching process and waiting until it's done)
			***********************************************************************************/
			else {
				pid_t pid = fork(); // Parent process gets pid of child assigned, child gets 0
				pid_t fg_pid, cpid, w;
				
				// If input file was given, open the input file for reading only and us it as input

				// If an output file was given, open the output file for reading only and use as output

				// If process was set to run as a bg process and no output file was given
				// then set the output of the bg process to dev/null

				// branch depending on if fork child or parent
				switch (pid) {
					case 0: // Child process -- attempt to execute command
						if(execvp(command, arguments)  == -1) {
							printf("Attempted to exec %s with ", command);
							print_array(arguments, arg_count);
							perror("smallsh");
							fg_exit_status = 1;
						}
						exit(1); // Only executes if execvp() failed
						break;

					case -1: // Fork() failed to create child, report the error
						perror("fork()");
						break;

					default: // Parent process will execute this code:
						// If BG mode, add its process id to the list, display pid to user
						if (bg_mode) {
							push_bg_pid(pid);
							printf("background pid is %d\n", pid);
						}

						// Foreground process, wait for it to finish, print the term signal
						else {
							// CITE: manpage for waitpid, code example on Ubuntu distro
			               // do {
			                   w = waitpid(cpid, &fg_exit_status, 0);
			                   // TODO: Figure out why this always exits to -1 instead of sending the termsig??
			               //     if (w == -1) {
			               //         perror("waitpid");
			               //         exit(EXIT_FAILURE); 
			               //     }

			               //     if (WIFEXITED(fg_exit_status)) {
			               //         printf("exited, status=%d\n", WEXITSTATUS(fg_exit_status));
			               //     } else if (WIFSIGNALED(fg_exit_status)) {
			               //         printf("killed by signal %d\n", WTERMSIG(fg_exit_status));
			               //     } else if (WIFSTOPPED(fg_exit_status)) {
			               //         printf("stopped by signal %d\n", WSTOPSIG(fg_exit_status));
			               //     } else if (WIFCONTINUED(fg_exit_status)) {
			               //         printf("continued\n");
			               //     }
			   //             // } while (!WIFEXITED(fg_exit_status) && !WIFSIGNALED(fg_exit_status));
						}
						break;
				} // end pid switch for fork()
			} // End Exec block
			command = NULL;
		 	free(arguments);
		} // end if (command = strtok...)
	}
}


// signal handler for SIGINT that does NOT terminate this shell,
// but instead terminates the foreground process
// Cite: TLPI Page 399
static void sig_handler(int sig) {
	// Restablish signal handlers for portability (without this, subsequent
	// CTRL+C call after first was causing smallsh to terminate)
	switch (sig) {
		case SIGINT:
			// Basically ignore interupt signal for the shell and restablish
			signal(SIGINT, sig_handler);
			printf("\n");
			break;

		case SIGCHLD:
			// Catch any child process  that ends and wait for it to kill the zombie
			signal(SIGCHLD, sig_handler);

			int exit_status;
			pid_t pid = waitpid(0, &exit_status, 0);
			// Print the string "background " if the process is in the bg process array
			if (pid != -1 && pid != 0) {
				if (is_bg_pid(pid)) {
					return;
				}

				if(WIFEXITED(exit_status)) {
					printf("pid %d is done: exit value %d\n", pid, WEXITSTATUS(exit_status));
				}
				// if it was terminated by signal, display the code:
				else if (WIFSIGNALED(exit_status)) {
					printf("pid %d is done: terminated by signal %d\n", pid, WTERMSIG(exit_status));
				}
			}
			break;
	}
	// if (sig == SIGINT) {
	// 	// Basically ignore interupt signal for the shell and restablish
	// 	signal(SIGINT, sig_handler);
	// 	printf("\n");
	// 	// command_prompt();
	// }
	// else {
	// 	printf("Signal: %d", sig);
	// }
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
	// Start up signal handler
	// Cite: TLPI Chapter 20, ~Page 401 and other examples
	init_bg_process_arr();
	signal(SIGINT, sig_handler);
	signal(SIGCHLD, sig_handler);

	command_prompt();
	return 0;
}