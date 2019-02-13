CFLAGS := ${CFLAGS} -g -O2 -Wall -Werror
acohttp: libaco/*.* *.h *.c
	$(CC) $(CFLAGS)  libaco/acosw.S libaco/aco.c *.c -o acohttp
