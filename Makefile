.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror -g -std=c99 -D_GNU_SOURCE
LDFLAGS = -lpthread
# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions
# LDFLAGS += -lsocket -lnsl -lrt

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,build/%.o,$(SRC))
DEP = $(patsubst %.c,build/%.d,$(SRC))
BIN = minishell

all: $(BIN)

build:
	@mkdir -p build

build/%.d: %.c | build
	@echo -n '$@ build/' > $@
	@$(CC) $(CFLAGS) -MM $< >> $@

build/csapp.o: csapp.c | build
	$(CC) -c $< -o $@

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	@rm -f $(BIN) $(DEP) $(OBJ)

-include $(DEP)