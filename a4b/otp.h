/*******************************************************************************
* File:         otp.h
* Author:       Shawn S Hillyer
* Date:         May, 2016
* Course:       OSU CSS 344-400: Assignment 04
* Description:  Include headers, constants, & common functions for OTP client.c
*               and server.c.
*               
*               
* Usage:        server <port_number>
*               Port must be in the range [MAX_PORT_NUMBER .. ]
*               
* Cite:         Overall flow of a socket-based client/server pair of programs: 
                beej.us/guide/bgipc/output/html/multipage/unixsock.html  
*******************************************************************************/

#ifndef SSHILLYER_OTP_H
#define SSHILLYER_OTP_H

#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MIN_PORT_NUMBER 1
#define MAX_PORT_NUMBER 65535
#define BUF_SIZE 1000000

/*******************************************************************************
* void check_argument_length(int arg_c, int req, const char * message)
* : validates that arg_c == req and prints message if not
* argc: number of arguments expected
* req: number of arguments required
* message: usage message
*******************************************************************************/
void check_argument_length(int arg_c, int req, const char * message) {
	if (arg_c != req) {
		fprintf(stderr, "%s", message);
		exit(EXIT_FAILURE);
	}
}


/*******************************************************************************
* void perror_exit(char * message, int exit_value) {
* prints message using perror() and calls exit with exit_value
* message: char * pointing to null terminated c-string
* exit_value: int representing the exit value to pass to exit
*******************************************************************************/
void perror_exit(char * message, int exit_value) {
	perror(message);
	exit(exit_value);
}

/*******************************************************************************
* confirm that port was parsed within valid range for ports
* Cite: man strtol, example section
*******************************************************************************/
void validate_port(int port, int err) {
	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) 
		|| (errno != 0 && port == 0) 
		|| (port > MAX_PORT_NUMBER || port < MIN_PORT_NUMBER)) 
	{
		perror_exit("strtol", EXIT_FAILURE);
	}
}

#endif

/*******************************************************************************
* 
* 
*******************************************************************************/