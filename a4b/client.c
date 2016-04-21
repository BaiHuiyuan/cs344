/*******************************************************************************
* File:         server.c
* Author:       Shawn S Hillyer
* Date:         May, 2016
* Course:       OSU CSS 344-400: Assignment 04
* Description:  
*               
*               
*               
* Usage:        
*               
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
                beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*******************************************************************************/

#include "otp.h"

int main(int argc, char const *argv[]) {
	
	// Verify Arguments are valid
	check_argument_length(argc, 2, "Usage: client [socket]\n");

	// parse port from command line argument and check result
	errno = 0; // Always set to 0 before a system call if checking; cite: manpage for errno.h
	int port = strtol(argv[1], NULL, 10);
	const char * port_str = argv[1];
	validate_port(port, errno);

	int sfd, cfd, status;  // listening socket file descriptor and connection file descriptor
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my ip for me

	if ( (status = getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
		perror_exit("getaddrinfo", EXIT_FAILURE);
	}

	// Now open a TCP socket stream; Cite: Slide 10 Unix Networking 2 (lecture)
	if ((sfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
		perror_exit("socket", EXIT_FAILURE);

	connect(sfd, servinfo->ai_addr, servinfo->ai_addrlen);

	char * message = "Hello from client\n";
	int len, bytes_sent;
	len = strlen(message);
	bytes_sent = send(sfd, message, len, 0);

	return 0;
}