/*******************************************************************************
* File:         otp_enc_d.c
* Author:       Shawn S Hillyer
* Date:         June 6, 2016
* Course:       OSU CSS 344-400: Assignment 04
*
* Description:  A daemon-like server process that listens on specified port for
*               a connection from otp_enc. If handshake succesful, attempts
*               to encrypt the plaintext using the key sent by client. Sends the
*               encrypted text back to the client.
*
* Usage:        otp_enc_d <port_number> &
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
	check_argument_count(argc, 2, "Usage: otp_enc_d port\n");


	// parse port from command line argument and check result
	// Even though we are using the string version of the port, validate as an int
	errno = 0; // 0 out before evaluating the call to strtol
	int port = strtol(argv[1], NULL, 10);
	validate_port(port, errno);
	const char * port_str = argv[1];

	// Variables for sockets and the server address
	int sfd, cfd, status;  
	ssize_t num_read, num_written; // # of bytes read
	char msg[BUF_SIZE];
	char key[BUF_SIZE];
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
	// TODO: See if this fits the requirement for the assignment as the pool or not?
	// TODO: I think we need to fork off but can ask on forums about this.
	if ( listen(sfd, 5) == -1)
		perror_exit("listen", EXIT_FAILURE);


	// Handle clients iteratively. 
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

			// Check for handshake message.
			const char * handshake_greeting = "otp_enc requests encryption";
			const char * handshake_response = "otc_enc_d confirms encryption";

			char handshake[BUF_SIZE];

			if ( (num_read = read(cfd, handshake, BUF_SIZE)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			} 

			// printf("DEBUG: server: Received this handshake: %s\n", handshake);
			
			if (strcmp(handshake, handshake_greeting) == 0 ) {
				// printf("DEBUG: server: Handshake request matches expected - attempting to write the response.\n");

				if (num_written = write(cfd, handshake_response, strlen(handshake_response)) == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}				
			} 
			else {
								// We silently (for now?) reject a bad handshake message
				exit(EXIT_FAILURE);
			}
			

			// Read the first write to socket - this should be the message
			if ( (num_read = read(cfd, msg, BUF_SIZE)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			// printf("DEBUG: server: received 'msg': %s\n", msg);

			// Read the second write to socket - this should be the key
			if ( (num_read = read(cfd, key, BUF_SIZE)) == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			// printf("DEBUG: server: received 'key': %s\n", key);

			char * resp = encrypt_string(msg, key, 0); // TODO: Make sure this is passed arg3 of '1' in decryption verison
			// printf("DEBUG: server: encrypt_string(msg, key) yields resp: %s\n", resp);

			// We write back the entire string plus a null byte so that the receiving end knows
			// when to null terminate its string!
			if (num_written = write(cfd, resp, strlen(resp) + 1) == -1) {
				perror("write");
				exit(EXIT_FAILURE);
			}

			// Close the connection after processing data
			if (close(cfd) == -1) {
				perror("close");
				continue;
			}

			free(resp);
		}
		else {
			// Parent process. GO back to top and listen some more
			continue;
		}
	} // end accept() loop

	return 0;
}