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

/*******************************************************************************
* main()
* Connects to a server and sends message
*******************************************************************************/
int main(int argc, char const *argv[]) {
	
	// Verify Arguments are valid
	check_argument_length(argc, 2, "Usage: client [socket]\n");


	// parse port from command line argument and check result
	// Even though we are using the string version of the port, validate as an int
	errno = 0; // 0 out before evaluating the call to strtol
	int port = strtol(argv[1], NULL, 10);
	validate_port(port, errno);
	const char * port_str = argv[1];


	// Variables for sockets and the server address
	int sfd, cfd, status; 
	struct addrinfo hints, *servinfo, *p;


	// 0 out hints struct then init to connect to localhost via TCP
	memset(&hints, 0, sizeof hints);
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


	// Finally, connect to server indicated by servinfo.ai_addr
	connect(sfd, servinfo->ai_addr, servinfo->ai_addrlen);


	// Send a message to the server.
	// TODO: Once have the assignment description, need to do the encryption or decryption
	char * message = "Hello from client\n";
	int len, bytes_sent;
	len = strlen(message);
	bytes_sent = send(sfd, message, len, 0);

	return 0;
}