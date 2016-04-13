/*******************************************************************************
* File:         smallsh.c
* Author:       Shawn S Hillyer
* Date:         , 2016
* Course:       OSU CSS 344-400: Assignment 03
* Description:  A small shell with 3 builtin commands (cd, status, exit)
*               Background a process by using & at end of command
*               Quotes not supported
*               Redirect input using < filename, output using > filename
Usage:        
  $ smallsh   # to launch program
  : command [arg1 arg2 ...] [< input_file] [> output_file] [&] # comments
*******************************************************************************/

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
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


/*******************************************************************************
 Struct for storing dynamic array of process id's running in background
 Cite: CS 261 - dynamic array materials.
 Decided to do this because I couldn't think of an easy way to keep a dynamic
 list of bg processes to review before each prompt to check if they had ended
 or not. Ultimately I figured out how to wait for children using wait() options
 However, when exiting shell, I still had to iterate over the list I maintained
 because i couldn't get wait() or waitpid() to wait until all children were done
 Ignoring traditional stance that "globals are bad" 
*******************************************************************************/
struct pid_arr {
	pid_t * pids; // array of process id's
	int capacity;
	int size;
};

// Make a single pid_arr in global scope
struct pid_arr bg_pids;

/*******************************************************************************
void init_bg_process_arr()
initialize a dynamic array of type pid_t. Uses global variable
*******************************************************************************/
void init_bg_process_arr() {
	bg_pids.capacity = 10;
	bg_pids.size = 0;
	bg_pids.pids = malloc(sizeof(pid_t) * bg_pids.capacity );
}

/*******************************************************************************
bool pid_arr_is_full()
Returns true if the dynamic pid array is full. Uses a global variable
*******************************************************************************/
bool pid_arr_is_full() {
	return (bg_pids.size >= bg_pids.capacity);
}

/*******************************************************************************
void push_bg_pid(pid_t pid)
push a pid onto end of dynamic array
*******************************************************************************/
void push_bg_pid(pid_t pid) {
	// If array is full, reallocate to twice size
	if (pid_arr_is_full()) {
		bg_pids.capacity *= 2;
		bg_pids.pids = realloc(bg_pids.pids, sizeof(pid_t) * bg_pids.capacity );
	}
	// TODO: if time, halve memory if less than 1/4 full
	bg_pids.pids[bg_pids.size++] = pid;
}

/*******************************************************************************
void remove_bg_pid(pid_t pid)
remove a pid from the bg_pids.pids[] array
*******************************************************************************/
void remove_bg_pid(pid_t pid) {
	int i, j;
	if (pid > 0) {
		// printf("removing pid %d\n", pid);
		for (i = 0; i < bg_pids.size; i++) {
			if (bg_pids.pids[i] == pid) {
				bg_pids.size--;
				for (j = i; j < bg_pids.size; j++)
					bg_pids.pids[j] = bg_pids.pids[j + 1];
			}
		}
	}
}

/*******************************************************************************
bool is_bg_pid(pid_t pid)
Return true if pid is in the bg_pids.pids array
*******************************************************************************/
bool is_bg_pid(pid_t pid) {
	int i;
	for (i = 0; i < bg_pids.size; i++) {
		if (bg_pids.pids[i] == pid)
			return true;
	}
	return false;
}

/*******************************************************************************
void kill_all_children()
sends SIGKILL to all background processes of the shell
Foreground processes wouldn't be running 
Cite: TLPI Section 20.5
*******************************************************************************/
void kill_all_children() {
	pid_t bg_process;
	int i;
	for (i = 0; i < bg_pids.size; i++) {
		kill(bg_pids.pids[i], SIGKILL); // kill all child processes
	}
}

/*******************************************************************************
void change_directory(char * dir) {
Change directory to dir. Supports relative & absolute paths because chdir does
*******************************************************************************/
void change_directory(char * dir) {
	if(!chdir(dir) == 0) {
		// chdir was unsuccesful
		perror("cd");
	}
}


/*******************************************************************************
void exit_shell()
Kill all child processes and free the memory used in global array
*******************************************************************************/
void exit_shell() {
	kill_all_children();
	free(bg_pids.pids);
	exit(0);
}

/*******************************************************************************
void check_bg_processes()
Recursive function. Checks (without blocking) if any child process has exited.
If so, prints the exit status or termination signal, removes from list of bg
processes, and recurisvely calls to check for additional. Base case is when 
no child processes have changed status (bg_pid == 0) or there are no child 
processes that are executing (-1)
*******************************************************************************/
void check_bg_processes() {
	// Cite: Slide 21 lecture 9, and the manpage for wait()
	// iterates through all the bg id's so that we don't print foreground process?
	pid_t bg_pid = -1;
	int i;
	int bg_exit_status;

	bg_pid = waitpid(-1, &bg_exit_status, WNOHANG);
	if (bg_pid != 0 && bg_pid != -1) {
		if (bg_pid > 0) {
			if (bg_exit_status !=0)
				printf("background pid %d is done: terminated by sig %d\n", bg_pid, bg_exit_status);
			else
				printf("background pid %d exited with code %d\n", bg_pid, bg_exit_status);
			remove_bg_pid(bg_pid);
		}
		// recursively check for additional processes, otherwise might overlook multiple finishing between prompts
		check_bg_processes(); 
	}
}


/*******************************************************************************
void command_prompt() {
REPL (Read, Evaluate, Print, Loop) function.
Reads user input, evaluates it by parsing the input, prints output, and repeats
Intercepts SIGINT and ignores; foreground processes do not ignore SIGINT, bg
processes will ignore them as well.
TODO: Would be nice to break this into smaller modules, but passing/updating
      the strings and variables across the functions ended up being error prone
*******************************************************************************/
void command_prompt() {
	bool repeat = true;
	int fg_exit_status = 0; // holds status of fg processes, used by 'status'
	const char * devnull = "/dev/null"; // Allocate statically; Used in redirection

	while(repeat) {
		// Variables used in parsing input	
		int arg_count = 0, 
			word_count = 0;
		char ** arguments = malloc(MAX_ARGS * sizeof(char *));
		char * words[MAX_ARGS + 1];
		char * command = NULL, * input_file = NULL, * output_file = NULL;
		char input[MAX_CHAR + 1];
		memset (input, '\0', MAX_CHAR);
		
		// Booleans to track redirection mode and background vs foreground
		bool bg_mode = false, redir_input = false, redir_output = false;
		
		// Establish a signal handler - catches SIGINT and ignores it. 
		// Restablished every loop b/c foreground process commands will re-enable 
		// the SIG_DFL (default action) setting when they launch
		// Cite: Slide 27 et. al. from lecture 13
		struct sigaction act;
		act.sa_handler = SIG_IGN;
		sigaction(SIGINT, &act, NULL);

		// List all bg processes that finished and status
		check_bg_processes();

		// Read: Prompt user, get input, null terminate string.
		printf(": ");
		fflush(stdout);
		fflush(stdin);
		fgets(input, MAX_CHAR, stdin);

		if (strlen(input) > 1) {
			// Lines that start with '#' are comments, jump to top of REPL
			if (input[0] == '#') {
				continue;
			}
			input[strlen(input)-1] = '\0'; // removes the newline and replaces it with null
		}

		// Check that the input was not null before evaluating further
		if (command = strtok(input, " \n")) {
			// store command as arguments[0] then move forward to tokenize rest
			arguments[arg_count++] = command;

			// Tokenize input, storing words until all words in words[] array
			// We are allowed to assume that the command is entered without syntax errors
			while(words[word_count] = strtok(NULL, " ")) {
				char * word = words[word_count]; // Vastly improves readability in this block
				
				// Ignore comments and rest of line if word starts with #
				if (word[0] == '#') {
					word[0] = '\0';
					break;
				}

				// set input redirection mode and filename
				if (strcmp(word, "<") == 0) {
					words[++word_count] = strtok(NULL, " ");
					input_file = words[word_count];
					redir_input = true;
				}
				
				// set output redirectino mode and filename
				else if (strcmp(word, ">") == 0) {
					words[++word_count] = strtok(NULL, " ");
					output_file = words[word_count];
					redir_output = true;
				}

				// Set BG mode, stop reading rest of line (must be last argument)
				else if (strcmp(word, "&") == 0) {
					bg_mode = true;
					break;
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
			* Non-builtins are executed by forking and calling execvp() which will check PATH
			* Cite: Slides from Lecture 9, especially, 28
			* Cite: brennan.io/2015/01/16/write-a-shell-in-c/  for idea on how to wait() for fg
			***********************************************************************************/
			else {
				// fork process, then set up in/out redirection as appropriate
				pid_t pid = fork(); // Parent process gets pid of child assigned, child gets 0
				pid_t wpid;
				int fd_in, fd_out, fd_in2, fd_out2;

				// Child process -- attempt to execute command
				if (pid == 0) {
					// Cite: Redirection pg 16-17
					// if we're in background mode and no input || output redirection was set, need
					// to send it to /dev/null file
					if (!redir_input && bg_mode)
						input_file = "/dev/null";
					if (!redir_output && bg_mode)
						output_file = "/dev/null";

					// Open input_file if not null (if it's null, then some file was provided or its foreground)
					if (input_file) {
						fd_in = open(input_file, O_RDONLY);
						if (fd_in == -1) {
							perror("open");
							exit(1);
						}
						fd_in2 = dup2(fd_in, 0); // 0 = stdin
						if (fd_in2 == -1) {
							perror("dup2");
							exit(2);
						}
					}

					// Open output_file if not null (if it's null, then some file was provided or its foreground)
					if (output_file) {
						fd_out = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
						if (fd_out == -1) {
							perror("open");
							exit(1);
						}

						fd_out2 = dup2(fd_out, 1); // 1 = stdout
						if (fd_out2 == -1) {
							perror("dup2");
							exit(2);
						}					
					}

					// Allow SIGINT if running in foreground
					if (!bg_mode) {
						act.sa_handler = SIG_DFL;
						act.sa_flags = 0;
						sigaction(SIGINT, &act, NULL);
					}

					fg_exit_status = execvp(command, arguments);

					// execvp only returns if an error has occured with value -1
					if(fg_exit_status == -1) {
						perror(command);
						fg_exit_status = 1; // exit status 1 per specs
					}
					exit(EXIT_FAILURE); 
				} // end child process
				
				// fork failed
				else if (pid == -1) {
					perror("fork()");
				}

				// Parent branch
				else {
					if (bg_mode) {
						// remember the pid but don't wait
						push_bg_pid(pid);
						printf("background pid is %d\n", pid);
					}
					else {
						// Foreground process - wait until done or termed
						// CITE: manpage for waitpid and lectures
	               		pid = waitpid(pid, &fg_exit_status, 0);

						if (pid != -1 && pid != 0) {
							// if it was terminated by signal, display the code:
							if (WIFSIGNALED(fg_exit_status)) {
								fg_exit_status = WTERMSIG(fg_exit_status); // So we can retrieve using status later
								printf("pid %d is done: terminated by signal %d\n", pid, fg_exit_status);
							}
							else if(WIFEXITED(fg_exit_status)) {
								fg_exit_status = WEXITSTATUS(fg_exit_status);
							}
						}
			        } // end foreground block
				} // end parent branch
			} // End Exec block
		} // end if (command = strtok...)
		if(arguments) free(arguments);	
	}
}

/*******************************************************************************
int main(int argc, char const *argv[])
sets up bg_pids dynamic array and begins command prompt for shell
*******************************************************************************/
int main(int argc, char const *argv[])
{
	// Cite: TLPI Chapter 20, ~Page 401 and other examples
	init_bg_process_arr();

	command_prompt();
	return 0;
}

// signal handler for SIGINT that does NOT terminate this shell,
// but instead terminates the foreground process
// Cite: TLPI Page 399
// static void sig_handler(int sig) {
// 	// Restablish signal handlers for portability (without this, subsequent
// 	// CTRL+C call after first was causing smallsh to terminate)
// 	// printf("Calling printf from inside a sig handler\n");
// 	switch (sig) {
// 		case SIGINT:
// 			// Basically ignore interupt signal for the shell and restablish
// 			signal(SIGINT, sig_handler);
// 			printf("\n");
// 			break;

// 		case SIGCHLD:
// 			// Catch any child process  that ends and wait for it to kill the zombie
// 			signal(SIGCHLD, sig_handler);

// 			int exit_status;
// 			pid_t pid = waitpid(0, &exit_status, 0);
// 			// Print the string "background " if the process is in the bg process array
// 			if (pid != -1 && pid != 0) {
// 				// if (is_bg_pid(pid)) {
// 				// 	return;
// 				// }

// 				if(WIFEXITED(exit_status)) {
// 					printf("pid %d is done: exit value %d\n", pid, WEXITSTATUS(exit_status));
// 				}
// 				// if it was terminated by signal, display the code:
// 				else if (WIFSIGNALED(exit_status)) {
// 					printf("pid %d is done: terminated by signal %d\n", pid, WTERMSIG(exit_status));
// 				}
// 			}
// 			break;
// 	}
// }


// Used for debugging purposes
// void print_array (char ** arguments, int n) {
// 	int i = 0;
// 	for (i = 0; i < n; i++) {
// 		printf("arg%d: %s\n", i, arguments[i]);
// 	}
// }

