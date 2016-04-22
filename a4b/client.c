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
* char * get_string_from_file(const char * fname) {
* 
* 
*******************************************************************************/
char * get_string_from_file(const char * fname) {
	FILE * file;
	if (!(file = fopen(fname, "r"))) {
		perror_exit("fopen", EXIT_FAILURE);
	}

	char * file_contents = malloc(sizeof (char) * BUF_SIZE);
	fgets(file_contents, BUF_SIZE - 1, file);
	char * return_string = malloc(sizeof(char) * strlen(file_contents));
	return_string = file_contents;

	if (file)
		fclose(file);

	return return_string;
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

	char * message = get_string_from_file(argv[1]);
	char * key = get_string_from_file(argv[2]);

	// Variables for sockets and the server address
	int sfd, status; 
	struct addrinfo hints, *servinfo;


	// 0 out hints struct then init to connect to localhost via TCP
	// Cite: lecture slides and man pages and beej guide
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
	// char * message = "Hello from client\n";
	

	int len, bytes_sent;
	len = strlen(message);
	bytes_sent = send(sfd, message, len, 0);
	bytes_sent = send(sfd, key, len, 0);



	// Free memory for message & key and addrinfo struct
	free(message);
	free(key);
	free(servinfo);

	return 0;
}