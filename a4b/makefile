CC = gcc

CFLAGS = -std=c99
#CFLAGS += -Wall
#CFLAGS += -g

LDFLAGS = -lm

SRC1 = otp_enc.c
SRC2 = otp_enc_d.c
SRC3 = otp_dec.c
SRC4 = otp_dec_d.c
SRC5 = keygen.c

SRCS = ${SRC1}

HEADER1 = otp.h
HEADERS = ${HEADER1}

PROG1 = otp_enc
PROG2 = otp_enc_d
PROG3 = otp_dec
PROG4 = otp_dec_d
PROG5 = keygen
PROGS = ${PROG1} ${PROG2} ${PROG3} ${PROG4} ${PROG5} 

DOC1 = plaintext1
DOC2 = plaintext2
DOC3 = plaintext3
DOC4 = plaintext4
DOC5 = plaintext5
DOCS = ${DOC1} ${DOC2} ${DOC3} ${DOC4} ${DOC5} ${DOC6}

ZIP = hillyers.otp.zip
	

default:
	${CC} ${CFLAGS} ${SRC1} ${HEADERS} -o ${PROG1} -D_XOPEN_SOURCE
	${CC} ${CFLAGS} ${SRC2} ${HEADERS} -o ${PROG2} -D_XOPEN_SOURCE
	${CC} ${CFLAGS} ${SRC3} ${HEADERS} -o ${PROG3} -D_XOPEN_SOURCE
	${CC} ${CFLAGS} ${SRC4} ${HEADERS} -o ${PROG4} -D_XOPEN_SOURCE
	${CC} ${CFLAGS} ${SRC5} ${HEADERS} -o ${PROG5} -D_XOPEN_SOURCE

zip:
	zip ${ZIP} ${SRCS} ${HEADERS} ${DOCS} makefile compileall

clean:
	rm -f ${PROGS} *.o *.~