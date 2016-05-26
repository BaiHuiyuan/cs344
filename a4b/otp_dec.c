/*******************************************************************************
* File:         otp_dec.c
* Author:       Shawn S Hillyer
* Date:         June 6, 2016
* Course:       OSU CSS 344-400: Assignment 04
*
* Description:  Connects to otp_dec_d on port and asks it to perform one time
*               pad decryption of plaintext using key.
*               
* Usage:        otp_dec plaintext key port
*               plaintext is a plaintext file to decrypt
*               key is a plaintext file used to decrypt using OTP method
*               port is the port number of server.c
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
	check_argument_count(argc, 4, "Usage: otp_dec message key port\n");

	// parse port from command line argument and check result
	
	// Even though we end up using string version of port, validate is valid int
	int port = convert_string_to_int(argv[3]);
	validate_port(port, errno);
	const char * port_str = argv[3];

	// Read the input files and verify key is long enough and no invalid characters
	char * message = get_string_from_file(argv[1]);
	char * key = get_string_from_file(argv[2]);

	validate_key_message_lengths(message, key, argv[2]);
	validate_characters(message);
	validate_characters(key);

	// Variables for sockets and the server address
	int sfd, status; 
	struct addrinfo hints, *servinfo;

	// 0 out hints struct then init to connect to localhost via TCP
	// Cite: lecture slides, man getaddrinfo(3), and beej guide - random bits
	// Use the getaddrinfo() to fill out servinfo by passing in some 'hints'
	memset(&hints, 0, sizeof hints);  // clear out the hints struct for safety
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM; // Use TCP -- need 2-way communication
	hints.ai_flags = AI_PASSIVE; // fill in localhost ip

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
		fprintf(stderr, "otp_dec: Could not contact otp_dec_d on port %d\n", port);
		exit(2);
	}


	// Variables for sending and receiving responses
	int len, bytes_sent, bytes_received;
	char resp[BUF_SIZE];
	

	// Send handshake greeting to server and verify talking to otp_dec_d
	const char * handshake_greeting = "otp_dec requests decryption";
	const char * handshake_response = "otc_dec_d confirms decryption";

	if(bytes_sent = send(sfd, handshake_greeting, strlen(handshake_greeting), 0) == -1) {
		perror_exit("send", EXIT_FAILURE);
		// fprintf(stderr, "otp_dec: send: err");
	}

	bytes_received = read(sfd, resp, BUF_SIZE);

	if (strcmp(resp, handshake_response) != 0 ) {
		fprintf(stderr, "ERROR: otp_dec: handshake: failed to handshake succesfully, connection refused", resp, handshake_response);
		exit(EXIT_FAILURE);
	}
	
	// Send the message and key to server for processing:
	// Message
	if(bytes_sent = send(sfd, message, strlen(message), 0) == -1)
		perror_exit("send", EXIT_FAILURE);

	// Key
	if(bytes_sent = send(sfd, key, strlen(key), 0) == -1)
		perror_exit("send", EXIT_FAILURE);

	// Receive response from the server and print to screen.
	if (bytes_received = read(sfd, resp, BUF_SIZE) == -1) {
		perror_exit("read", EXIT_FAILURE);
	}

	// TODO: Ask instructor if the program should also output a newline at end??
	fprintf(stdout, "%s", resp); 

	// Do some cleanup	
	cleanup_memory(message, key, servinfo);	

	return 0;
}