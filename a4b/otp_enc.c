/*******************************************************************************
* File:         otp_enc.c
* Author:       Shawn S Hillyer
* Date:         June 6, 2016
* Course:       OSU CSS 344-400: Assignment 04
*
* Description:  Connects to otp_enc_d on port and asks it to perform one time
*               pad encryption of plaintext using key.
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
* main()
* Connects to a server and sends message
*******************************************************************************/
int main(int argc, char const *argv[]) {
	
	// Verify Arguments are valid
	check_argument_count(argc, 4, "Usage: otp_enc message key port\n");
	
	// Parse and validate port, save port as a string for loading address
	int port = convert_string_to_int(argv[3]);
	validate_port(port, errno);
	const char * port_str = argv[3];

	// Read string from files, validate lengths and characters meet requirements
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
		fprintf(stderr, "otp_enc: Could not contact otp_enc_d on port %d\n", port);
		exit(2);
	}


	// Variables for sending and receiving responses
	long len, bytes_sent, bytes_received, bytes_remaining;
	char resp[BUF_SIZE];
	
	// Send handshake greeting to server and verify talking to otp_enc_d
	char * handshake_greeting = "otp_enc requests encryption";
	char * handshake_response = "otc_enc_d confirms encryption";

	// Send handshake
	safe_transmit_msg_on_socket(sfd, handshake_greeting, strlen(handshake_greeting), 2);

	// Receive response
	// safe_transmit_msg_on_socket(sfd, resp, BUF_SIZE, 1);
	bytes_received = read(sfd, resp, BUF_SIZE);

	if (strcmp(resp, handshake_response) != 0 ) {
		// This will print whatever respones the server sends (if wasn't correct handshake)
		fprintf(stderr, "ERROR: %s", resp);
		exit(EXIT_FAILURE);
	}
	
	// Send Message length
	long message_length = strlen(message);
	
	if (bytes_sent = send(sfd, &message_length, sizeof(long), 0) == -1)
		perror_exit("send", EXIT_FAILURE);

	// Send Message
	safe_transmit_msg_on_socket(sfd, message, message_length, 2);

	// Send Key
	safe_transmit_msg_on_socket(sfd, key, message_length, 2);

	// Receive response from the server and print to screen.
	safe_transmit_msg_on_socket(sfd, resp, message_length, 1);

	// Write the message to the stdout filestream as bytes
	// Note: This is a lot easier than storing a nullterminated string and
	// the main reason I got the message_length ahead of time
	write(STDOUT_FILENO, resp, message_length);
	write(STDOUT_FILENO, "\n", 1);

	// Free the dynamic allocated memory we used
	cleanup_memory(message, key, servinfo);	

	return 0;
}