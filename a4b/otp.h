/*******************************************************************************
* File:         otp.h
* Author:       Shawn S Hillyer
* Date:         June 6, 2016
* Course:       OSU CSS 344-400: Assignment 04
*
* Description:  Include headers, constants, & common functions for OTP client
*               and server programs.
*               
* Usage:        #include <"otp.h">
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
                beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*******************************************************************************/

#ifndef SSHILLYER_OTP_H
#define SSHILLYER_OTP_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


// Constants
#define MIN_PORT_NUMBER 1
#define MAX_PORT_NUMBER 65535
#define BUF_SIZE 1000000
#define NUM_CH 27


/*******************************************************************************
* void check_argument_count(int arg_c, int req, const char * message)
* : validates that arg_c == req and prints message if not
* argc: number of arguments expected
* req: number of arguments required
* message: usage message
*******************************************************************************/
void check_argument_count(int arg_c, int req, const char * message) {
	if (arg_c != req) {
		fprintf(stderr, "%s", message);
		exit(EXIT_FAILURE);
	}
}


/*******************************************************************************
* void perror_exit(char * message, int exit_value)
* prints message using perror() and calls exit with exit_value
* message: char * pointing to null terminated c-string
* exit_value: int representing the exit value to pass to exit
*******************************************************************************/
void perror_exit(char * message, int exit_value) {
	perror(message);
	exit(exit_value);
}


/*******************************************************************************
* int convert_string_to_int(const char * string)
* string: A null terminated string to be converted to an integer.
* Takes a string argument and returns best representation of it as a string
*******************************************************************************/
int convert_string_to_int(const char * string) {
	// parse port from command line argument and check result
	// Even though we are using the string version of the port, validate as an int
	errno = 0; // 0 out before evaluating the call to strtol
	int result = strtol(string, NULL, 10);
	if (errno == ERANGE) {
		fprintf(stderr, "The string '%s' converts to an integer outside the valid range.\n", string);
		exit(EXIT_FAILURE);
	}
	return result;
}


/*******************************************************************************
* void validate_port(int port, int err)
* confirm that port was parsed within valid range for ports
* Cite: man strtol, example section
*******************************************************************************/
void validate_port(int port, int err) {
	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) 
		|| (errno != 0 && port == 0) 
		|| (port > MAX_PORT_NUMBER || port < MIN_PORT_NUMBER)) 
	{
		perror_exit("strtol", EXIT_FAILURE);
	}
}

/*******************************************************************************
* void strip_newline_from_string(char * string)
* string: A null-terminated c-string
* Strips newline, if one exists, from string & replace with null terminator
*******************************************************************************/
void strip_newline_from_string(char * string) {
	/* strcspn returns the length of all characters in a string that are not in
	the list of characters in the second argument. So this gives us the end
	and we place a null at that location in the string. */
	string[strcspn(string, "\r\n")] = 0; // replace LF, CR, CRLF< LFCR with null
}



/*******************************************************************************
* validate_key_message_lengths(char * msg, key)
* msg: string to compare
* key: string to compare
* key_filename: Source of the filename the key string was in.
* Ensure that the key is at least asl big as the msg
*******************************************************************************/
void validate_key_message_lengths(char * msg, char * key, const char * key_filename) {
	if (strlen(key) < strlen(msg)) {
		fprintf(stderr, "Error: key '%s' is too short\n", key_filename);
		exit(EXIT_FAILURE);
	}
}


/*******************************************************************************
* validate_characters(char * string)
* string: string to examine for valid characters
* Check that all of the characters in the string are valid. For this program,
* valid characters include [A-Z] and the space character. Newlines have already
* been stripped from the strings so don't have to check for those.
*******************************************************************************/
void validate_characters(char * string) {
	int len = strlen(string);
	for (int i = 0; i < len; ++i) {
		char cur = string[i];
		if (!isupper(cur) && !isspace(cur)) {
			fprintf(stderr, "error: input contains bad characters\n");
			exit(1);
		}
	}
}

/*******************************************************************************
* char * get_string_from_file(const char * fname)
* fname: string of a filename to read from
* Reads the first line of a file and return the malloc'd str to caller
*******************************************************************************/
char * get_string_from_file(const char * fname) {
	// Open file, read first line, close file, and return string
	FILE * file;
	if (!(file = fopen(fname, "r"))) {
		perror_exit("fopen", EXIT_FAILURE);
	}

	// fgets() reads until newline or '\0', then strip it off
	char * file_contents = malloc(sizeof (char) * BUF_SIZE);
	fgets(file_contents, BUF_SIZE - 1, file);
	strip_newline_from_string(file_contents);
	
	if (file)  // Close file if safely (failsafe, should have exited if NULL)
		fclose(file);

	return file_contents;
}


/*******************************************************************************
* int char_to_int(char ch)
* ch: char to convert to an integer representation
* Converts a character from [A-Z] and the space char into int in range [0-27]
*******************************************************************************/
int char_to_int(char ch) {
	if (isspace(ch)) {
		return 26; // 0 = A .. 25 = Z, 26 = ' '
	} 
	else {
		return (ch - 'A');
	}
}


/*******************************************************************************
* void cleanup_memory(char * str1, char * str2, struct addrinfo * addr) 
* cleans up dynamic memory for the two strings and the addrinfo struct
*******************************************************************************/
void cleanup_memory(char * str1, char * str2, struct addrinfo * addr) {
	if(str1) free(str1);
	if(str2) free(str2);
	// Per getaddrinfo() and Beej's NG, make sure to always call freeaddrinfo :)
	if(addr) freeaddrinfo(addr);
}


/*******************************************************************************
* char * encrypt_string(char * str)
* str: A string to encrypt (or decrypt)
* msg: A message to encrypt
* key: A string at least as long as the message to use as the key for encryption
* reverse_encryption_mode: 0 (false) or 1 (true) to indicate if should encrypt
*     or decrypt msg
* Encrypts 'msg' using OTP encryption with string held in 'key'
*******************************************************************************/
char * encrypt_string(char * msg, char * key, int reverse_encryption_mode) {
	// Convert ascii values from ['A'-'Z'] to [0 - 26] and ' ' to 27, then map
	// each character to an index in array of valid chars and assign to new str
	const char charmap[NUM_CH] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	
	char * str_encr = malloc (sizeof (char) * strlen(msg) + 1);
	int i = 0;

	for (int i = 0; i < strlen(msg); i++) {
		// Convert letters in the strings to int representation
		// Use the char/integer after 'Z' to represent the space char
		int msg_ch_val = char_to_int(msg[i]);
		int key_ch_val = char_to_int(key[i]);
		int ch_index;

		// Add or subtract the values and keep within valid range
		if (reverse_encryption_mode) {
			ch_index = (msg_ch_val - key_ch_val) % NUM_CH;
			if (ch_index < 0)
				ch_index += NUM_CH;
		}
		else {
			ch_index = (msg_ch_val + key_ch_val) % NUM_CH;
		}

		// Assign the characters into the return string by referencing the map
		str_encr[i] = charmap[ch_index];
	}

	// null terminate the string and return the result.
	str_encr[strlen(msg) + 1] = '\0'; 
	
	return str_encr;
}

/*******************************************************************************
* void safe_transmit_msg_on_socket(int message_length, int fd, char * buffer, int mode) {
* 
*******************************************************************************/
void safe_transmit_msg_on_socket(int fd, char * buffer, int message_length, int mode) {
	// mode == 1 : read | mode == 2 : write
	int bytes_remaining = message_length, bytes_transmitted = 0;
	while ( bytes_remaining > 0 ) {
		int i = message_length - bytes_remaining;
		if (mode == 1)
			bytes_transmitted = read(fd, buffer + i, bytes_remaining);
		else if (mode == 2)
			bytes_transmitted = write(fd, buffer, bytes_remaining);
		bytes_remaining -= bytes_transmitted;
	}
	if (bytes_transmitted == -1) {
		if (mode == 1)
			perror("read");
		else if (mode == 2)
			perror("write");	
		exit(EXIT_FAILURE);
	}
}


#endif