# Makefile for WMM

CC = gcc
CFLAGS = -g -O2 -Wall -W -fPIC
LDFLAGS = -lm
LIBFLAGS = -static

# Library name
LIBNAME = libwmm

# Executable name
BINNAME = wmm_point

LIBSRCFILES = WMM_SubLibrary.c
LIBOBJFILES = ${LIBSRCFILES:.c=.o}

BINSRCFILES = wmm_point.c
BINOBJFILES = ${BINSRCFILES:.c=.o}

all: bin

lib: ${LIBNAME}.a ${LIBNAME}.so
	ar rcs ${LIBNAME}.a ${LIBOBJFILES}
	${CC} -shared -Wl,-soname,${LIBNAME}.so.1 -o ${LIBNAME}.so ${LIBOBJFILES}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

${LIBNAME}.a: ${LIBOBJFILES}
	ar rcs ${LIBNAME}.a ${LIBOBJFILES}

${LIBNAME}.so: ${LIBOBJFILES}
	${CC} -shared -Wl,-soname,${LIBNAME}.so.1 -o ${LIBNAME}.so ${LIBOBJFILES}

clean:
	rm -f *.o ${LIBNAME}.*

bin: lib ${BINOBJFILES}
	${CC} -o ${BINNAME} ${BINOBJFILES} ${LIBNAME}.a ${LDFLAGS}
