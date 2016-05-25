#include "otp.h"
#include <time.h>

int main(int argc, char const *argv[]) {
	// Verify Arguments are valid
	check_argument_length(argc, 2, "Usage: keygen keylength\n");

	// parse keylength from commandline and make sure it's a valid integer
	errno = 0; // 0 out before evaluating the call to strtol
	int keylength = strtol(argv[1], NULL, 10);

	if (keylength == 0) {
		fprintf(stderr, "keygen: error parsing keylength, must be digit-string\n");
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