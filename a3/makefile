CC = gcc

CFLAGS = -std=c99
#CFLAGS += -Wall
#CFLAGS += -g

LDFLAGS = -lm

SRC1 = smallsh.c

SRCS = ${SRC1}

HEADER1 = 
HEADERS = ${HEADER1}

PROG1 = smallsh
PROGS = ${PROG1}

DOC1 = readme.txt
DOCS = ${DOC1}

ZIP = hillyers-a3.zip
	

default:
	${CC} ${CFLAGS} ${SRCS} ${HEADERS} -o ${PROGS} -D_XOPEN_SOURCE

zip:
	zip ${ZIP} ${SRCS} ${HEADERS} ${DOCS} makefile

clean:
	rm -f ${PROGS} *.o *.~