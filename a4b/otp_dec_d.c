/*******************************************************************************
* File:         otp_dec_d.c
* Author:       Shawn S Hillyer
* Date:         June 6, 2016
* Course:       OSU CSS 344-400: Assignment 04
*
* Description:  A daemon-like server process that listens on specified port for
*               a connection from otp_dec. If handshake succesful, attempts
*               to decrypt the ciphertext using the key sent by client. Sends the
*               decrypted text back to the client.
*
* Usage:        otp_dec_d <port_number> &
*               Port must be in the range [MAX_PORT_NUMBER .. ]
*               Should always run in background mode
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
*               beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*******************************************************************************/

#include "otp.h"


/*******************************************************************************
* main()
* Creates a socket and listens for incoming connections
*******************************************************************************/
int main(int argc, char const *argv[]) {
	
	// Verify Arguments are valid
	check_argument_count(argc, 2, "Usage: otp_dec_d port\n");

	// parse port from command line argument and check result
	// Even though we are using the string version of the port, validate as an int
	errno = 0; // 0 out before evaluating the call to strtol
	int port = strtol(argv[1], NULL, 10);
	validate_port(port, errno);
	const char * port_str = argv[1];


	// Variables for sockets and the server address
	int sfd, cfd, status;  
	long num_read, num_written; // # of bytes read
	struct addrinfo hints, *servinfo;// , *p;


	// Initialize struct sockaddr_in before making and binding socket
	// Cite: lecture slides and man pages and beej guide
	memset(&hints, 0, sizeof hints); // clear structure
	hints.ai_family = AF_INET; // AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my ip for me

	// populate servinfo using the hints struct
	if ( (status = getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
		perror_exit("getaddrinfo", EXIT_FAILURE);
	}


	// Now open a TCP socket stream; Cite: Slide 10 Unix Networking 2 (lecture)
	// Cite: Beej network guide for using hints structure
	if ((sfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
		perror_exit("socket", EXIT_FAILURE);


	// Bind the server socket
	if ( bind(sfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		// printf("sfd: %d\t servinfo->ai_addr: %d\t sizeof: %d\n", sfd, servinfo->ai_addr, sizeof(servinfo->ai_addrlen));
		perror_exit("bind", EXIT_FAILURE);
	}
	

	// listen for up to 5 connections in queue
	if ( listen(sfd, 5) == -1)
		perror_exit("listen", EXIT_FAILURE);


	// Handle clients using forks
	// Cite: TLPI pg 1168 and beej guide, Slide 12+ of lecture 17
	while (1) {
		// Accept a connection on a new socket (cfd) from queue
		// Will block until a connection comes in.
		if ( (cfd = accept(sfd, NULL, NULL)) == -1) {
			perror("accept");
			continue;
		}

		pid_t pid = fork();

		if (pid == 0) {
			// child process -- handle the connection
			char ciphertext[BUF_SIZE];
			char key[BUF_SIZE];		

			// Check for handshake message.
			const char * handshake_greeting = "otp_dec requests decryption";
			const char * handshake_response = "otc_dec_d confirms decryption";

			char handshake[BUF_SIZE];

			if ( (num_read = read(cfd, handshake, BUF_SIZE)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			} 
			
			if (strcmp(handshake, handshake_greeting) == 0 ) {
				// Handshake succesfull, write back success response
				if (num_written = write(cfd, handshake_response, strlen(handshake_response)) == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}				
			} 
			else {
				// Handshake failed, write back fail response
				char * fail_response = "otp_dec_d rejects otp_enc handshake";
				if (num_written = write(cfd, fail_response, strlen(fail_response)) == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_FAILURE);
			}
			
			// Receive from client the number of bytes (message_length)
			long message_length = 0;
			if ( (num_read = read(cfd, &message_length, sizeof(long))) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}

			// Read the first write to socket - this should be the ciphertext
			if ( (num_read = read(cfd, ciphertext, message_length)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}

			// Read the second write to socket - this should be the key
			// Note that we can ignore extraneous characters in the key beyond message_length
			if ( (num_read = read(cfd, key, message_length)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}

			// Generate a response as a string of characters
			char * resp = encrypt_string(ciphertext, key, 1);
			

			// Write back the string, stopping at message_length characters
			if (num_written = write(cfd, resp, message_length) == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}

			// Close the connection after processing data
			if (close(cfd) == -1) {
				perror("close");
				exit(EXIT_FAILURE);
			}

			free(resp);
			exit(0);
		} 
		else {
			// Parent process. GO back to top and listen some more
			continue;
		}
	} // end accept() loop
	
	return 0;
}