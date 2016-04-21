// otp.h
#ifndef SSHILLYER_OTP_H
#define SSHILLYER_OTP_H

#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
//#include <sys/un.h> // Used for struct sockaddr_un
#include <unistd.h>

#define BUF_SIZE 100

void check_argument_length(int argc, int req, const char * message) {
	if (argc != req) {
		fprintf(stderr, "%s", message);
		exit(EXIT_FAILURE);
	}
}

void perror_exit(char * message, int exit_value) {
	perror(message);
	exit(exit_value);
}

void validate_port(int port, int errno) {
	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) || (errno != 0 && port == 0) || (port > 65535 || port < 0)) {
			perror_exit("strtol", EXIT_FAILURE);
	}
}


#endif