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
	int repeat = 1;
	int fg_exit_status;
	const char * devnull = "/dev/null";

	while(repeat == 1) {
		
	char input[MAX_CHAR+1]; 
		check_bg_processes();

		printf(": ");
		fgets(input, sizeof(input), stdin);
		fflush(stdout);
		fflush(stdin);

		if (strlen(input) > 0)
			input[strlen(input)-1] = '\0';

		int arg_count = 0, 
			word_count = 0;
		char ** arguments = malloc(MAX_ARGS * sizeof(char *));
		char * words[MAX_ARGS + 1];
		char * command = NULL;
		char * input_file = NULL;
		char * output_file = NULL;
		
		bool bg_mode = false;
		bool redir_input = false;
		bool redir_output = false;

		if (input[0] == '#') {
			continue;
		}
		
		if (command = strtok(input, " ")) {
			arguments[arg_count++] = command;

			while(words[word_count] = strtok(NULL, " ")) {
				char * word = words[word_count]; // Vastly improves readability in this block
				
				if (word[0] == '#') {
					word[0] = '\0';
					break;
				}

				if (strcmp(word, "<") == 0) {
					words[++word_count] = strtok(NULL, " ");
					input_file = words[word_count];
					redir_input = true;
				}
				
				else if (strcmp(word, ">") == 0) {
					words[++word_count] = strtok(NULL, " ");
					output_file = words[word_count];
					redir_output = true;
				}

				else if (strcmp(word, "&") == 0) {
					bg_mode = true;
					if (words[++word_count] = strtok(NULL, " ")) {
						printf("Usage: '&' must be last word of the command\n");
						break;
					}
				}

				else {
					arguments[arg_count++] = words[word_count++];
				}
			} // end parsing data block

			arguments[arg_count] = NULL;

			/***********************************************************************************
			* Built-in command evaluation. These do NOT have to worry about redirection
			***********************************************************************************/
			
			if (strcmp(command, "exit") == 0) {
				free(arguments);
				exit_shell();
			}

			else if (strcmp(command, "cd") == 0) {
				if (arg_count == 1) {
					change_directory(getenv("HOME"));
				}

				else if (arg_count == 2) {
					change_directory(arguments[1]);
				}

				else {
					printf("smallsh: cd: usage: cd [directory]\n");
				}
			}

			else if (strcmp(command, "status") == 0) {
				printf("exit value %d\n", fg_exit_status);
			}

			else {
				pid_t pid = fork(); // Parent process gets pid of child assigned, child gets 0
				pid_t fg_pid, cpid, w;

				switch (pid) {
					case 0: // Child process -- attempt to execute command
						printf("pid %d: Child. Attempt to exec %s with \n", getpid(), command);
						print_array(arguments, arg_count);
						execvp(command, arguments);
						perror("smallsh");
						fg_exit_status = 1;
						exit(EXIT_FAILURE); // Only executes if execvp() failed
						printf("I should never see this.\n");
						return;

					case -1: // Fork() failed to create child, report the error
						perror("fork()");
						break;

					default: // Parent process will execute this code:
						printf("pid %d: Entering default\n", getpid());

						if (bg_mode) {
							printf("pid %d: if (bg_mode) block\n", getpid());
							push_bg_pid(pid);
							printf("pid %d: background pid is %d\n", getpid(), pid);
						}

						else {
							printf("pid %d: else block. foreground process executes this\n", getpid());
		                   w = waitpid(cpid, &fg_exit_status, 0);
						}
						break;
				} // end pid switch for fork()
			} // End Exec block
			command = NULL;
		 	free(arguments);
		} // end if (command = strtok...)
	}
}


static void sig_handler(int sig) {
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