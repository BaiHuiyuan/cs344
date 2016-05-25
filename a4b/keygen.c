/*******************************************************************************
* File:         keygen.c
* Author:       Shawn S Hillyer
* Date:         June 6, 2016
* Course:       OSU CSS 344-400: Assignment 04
*
* Description:  Generates a random sequence of capital letters / space characters
*               to use with a One Time Pad encryption program.
*               Sequence terminated with a newline.
*               Output is sent to stdout, errors to stderr
*               
* Usage:        keygen keylength
*               keylength is a positive integer small enough to fit in long int
*               
* Cite:         No outside sources referenced.
*******************************************************************************/

#include "otp.h"
#include <time.h>

int main(int argc, char const *argv[]) {
	// Verify Arguments are valid
	check_argument_count(argc, 2, "Usage: keygen keylength\n");

	// parse keylength from commandline and make sure it's a valid integer
	int keylength = convert_string_to_int(argv[1]);

	if (keylength <= 0) {
		fprintf(stderr, "keygen: error parsing keylength, must be a positive integer.\n");
		exit(EXIT_FAILURE);
	}

	// Rather than do some complex modulo operation/function, just map all integers from
	// 0 to 27 to the valid characters (with SPACE being the last of these);
	const char character_map[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	srand(time(NULL));

	for (int i = 0; i < keylength; i++) {
		int ch_index = rand() % 27;
		fprintf(stdout, "%c", character_map[ch_index]);
	}

	// add the trailing newline because reasons (plaintext files need them at end)
	fprintf(stdout, "\n"); 

	return 0;
}