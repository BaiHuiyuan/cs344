CC = gcc

CFLAGS = -std=c99
#CFLAGS += -Wall
#CFLAGS += -g

LDFLAGS = -lm

SRC1 = client.c
SRC2 = server.c

SRCS = ${SRC1}

HEADER1 = 
HEADERS = ${HEADER1}

PROG1 = client
PROG2 = server
PROGS = ${PROG1}

DOC1 = readme.txt
DOCS = ${DOC1}

ZIP = hillyers.otp.zip
	

default:
	#${CC} ${CFLAGS} ${SRC1} ${HEADERS} -o ${PROG1} -D_XOPEN_SOURCE
	${CC} ${CFLAGS} ${SRC2} ${HEADERS} -o ${PROG2} -D_XOPEN_SOURCE


zip:
	zip ${ZIP} ${SRCS} ${HEADERS} ${DOCS} makefile

clean:
	rm -f ${PROGS} *.o *.~