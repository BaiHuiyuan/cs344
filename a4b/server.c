/*******************************************************************************
* File:         server.c
* Author:       Shawn S Hillyer
* Date:         May, 2016
* Course:       OSU CSS 344-400: Assignment 04
* Description:  
*               
*               
*               
* Usage:        server <port_number>
*               Port must be in the range [MAX_PORT_NUMBER .. ]
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
                beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*******************************************************************************/

#include "otp.h"

/*******************************************************************************
* main()
* Creates a socket and listens for incoming connections
*******************************************************************************/
int main(int argc, char const *argv[]) {
	
	// Verify Arguments are valid
	check_argument_length(argc, 2, "Usage: server [socket]\n");


	// parse port from command line argument and check result
	errno = 0; // 0 out before evaluating the call to strtol
	int port = strtol(argv[1], NULL, 10);
	validate_port(port, errno);


	// Variables for sockets and the server address
	int sfd, cfd;  
	ssize_t num_read; // # of bytes read
	char buf[BUF_SIZE];
	struct sockaddr_in server;


	// Initialize struct sockaddr_in before making and binding socket
	// Cite: lecture slides and man pages and beej guide
	memset(&server, 0, sizeof(struct sockaddr_in)); // clear structure
	server.sin_family = AF_INET;
	server.sin_port = htons(port); // Ensure port is stored in network byte order
	server.sin_addr.s_addr = INADDR_ANY;

	
	// Now open a TCP socket stream; Cite: Slide 10 Unix Networking 2 (lecture)
	if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		perror_exit("socket", EXIT_FAILURE);


	// Bind the server socket
	if ( bind(sfd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1)
		perror_exit("bind", EXIT_FAILURE);
	

	// listen for up to 5 connections in queue
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

		// Echo data from connected socket to stdout until EOF
		while ( (num_read = read(cfd, buf, BUF_SIZE)) > 0)
			// Die a cruel death if the number written doesn't match the number read
			if (write(STDOUT_FILENO, buf, num_read) != num_read)
				perror_exit("partial/failed write", EXIT_FAILURE);
		
		// IF read failed, die 
		if (num_read == -1)
			perror_exit("read", EXIT_FAILURE);

		// otherwise read() has reached EOF. close connection socket ("cfd")
		if (close(cfd) == -1)
			perror_exit("close", EXIT_FAILURE);
	}

	// TODO: Are open socket fd's closed automatically on program termination?
	// TODO: Would we need to trap interupt or other signals? Not running from a fork so if it's in BG... no?
	return 0;
}