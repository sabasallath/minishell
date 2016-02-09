.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror -g
LDFLAGS =

# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions 
#LIBS += -lsocket -lnsl -lrt
LIBS += -lpthread


all: myshell	

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c -o $@ $<	

parseline.o: parseline.c myshell.h
	$(CC) $(CFLAGS) -c -o $@ $<

eval.o: eval.c myshell.h
	$(CC) $(CFLAGS) -c -o $@ $<

myshell.o: myshell.c myshell.h
	$(CC) $(CFLAGS) -c -o $@ $<	

jobs.o: jobs.c jobs.h
	$(CC) $(CFLAGS) -c -o $@ $<	

myshell: myshell.o eval.o parseline.o csapp.o jobs.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -f myshell *.o
