/*******************************************************************************
* File:         otp_enc.c
* Author:       Shawn S Hillyer
* Date:         June 6, 2016
* Course:       OSU CSS 344-400: Assignment 04
*
* Description:  Connects to otp_enc_d on port and asks it to perform one time
*               pad encryption of plaintext using key.
*               
*               
* Usage:        otp_enc plaintext key port
*               plaintext is a plaintext file to encrypt
*               key is a plaintext file used to encrypt using OTP method
*               port is the port number of server.c
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
                beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*******************************************************************************/

#include "otp.h"

/*******************************************************************************
* void strip_newline_from_string(char * string) {
* Strips newline, if one exists, from string & replace with null terminator
*******************************************************************************/
void strip_newline_from_string(char * string) {
	string[strcspn(string, "\r\n")] = 0; // replace LF, CR, CRLF< LFCR with null
}

/*******************************************************************************
* char * get_string_from_file(const char * fname) {
* Reads the first line of a file and return the malloc'd str to caller
*******************************************************************************/
char * get_string_from_file(const char * fname) {
	// Open file, read first line, close file, and return string
	FILE * file;
	if (!(file = fopen(fname, "r"))) {
		perror_exit("fopen", EXIT_FAILURE);
	}

	char * file_contents = malloc(sizeof (char) * BUF_SIZE);
	fgets(file_contents, BUF_SIZE - 1, file);
	strip_newline_from_string(file_contents);
	
	if (file)  // Close file if safely (failsafe, should have exited if NULL)
		fclose(file);

	return file_contents;
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
* main()
* Connects to a server and sends message
*******************************************************************************/
int main(int argc, char const *argv[]) {
	
	// Verify Arguments are valid
	check_argument_length(argc, 4, "Usage: client message key port\n");


	// parse port from command line argument and check result
	// Even though we are using the string version of the port, validate as an int
	errno = 0; // 0 out before evaluating the call to strtol
	int port = strtol(argv[3], NULL, 10);
	validate_port(port, errno);
	const char * port_str = argv[3];

	// Before bothering with sockets, attempt to read from message and key files
	char * message = get_string_from_file(argv[1]);
	char * key = get_string_from_file(argv[2]);

	// Variables for sockets and the server address
	int sfd, status; 
	struct addrinfo hints, *servinfo;


	// 0 out hints struct then init to connect to localhost via TCP
	// Cite: lecture slides, man getaddrinfo(3), and beej guide - random bits
	// Use the getaddrinfo() to fill out servinfo by passing in some 'hints'
	memset(&hints, 0, sizeof hints);  // clear out the hints struct for safety
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM; // Use TCP -- need 2-way communication
	hints.ai_flags = AI_PASSIVE; // fill in local ip

	// populate servinfo using the hints struct
	if ( (status = getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
		cleanup_memory(message, key, servinfo);	
		perror_exit("getaddrinfo", EXIT_FAILURE);
	}


	// Now open a TCP socket stream; Cite: Slide 10 Unix Networking 2 (lecture)
	// Cite: Beej network guide for using hints structure
	// Must be called after getaddrinfo() so that servinfo struct is populated
	if ((sfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		cleanup_memory(message, key, servinfo);	
		perror_exit("socket", EXIT_FAILURE);
	}


	// Connect to server indicated by servinfo.ai_addr
	if(connect(sfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		cleanup_memory(message, key, servinfo);	
		perror_exit("connect", EXIT_FAILURE);
	}


	// Send the message and key to server for processing	
	int len, bytes_sent, bytes_received;

	// Message
	len = strlen(message);
	if(bytes_sent = send(sfd, message, len, 0) == -1)
		perror_exit("send", EXIT_FAILURE);

	// Key 
	len = strlen(key);
	if(bytes_sent = send(sfd, key, len, 0) == -1)
		perror_exit("send", EXIT_FAILURE);

	// Receive response from the server and print to screen.
	char resp[BUF_SIZE];
	bytes_received = read(sfd, resp, BUF_SIZE); // TODO ERROR CHECKING
	printf("DEBUG: client: received 'response': %s\n", resp);


	// Do some leanup	
	cleanup_memory(message, key, servinfo);	

	return 0;
}