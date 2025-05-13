CC = gcc
CFLAGS = -Wall -Wextra -g -I.. -I. -I../include -I../my_libc
LDFLAGS = -lpcap -pthread

# Test source files
TEST_SRCS = test_framework.c \
            unit/test_cli_parser.c

# Implementation files
IMPL_SRCS = ../source/cli/parser_cli_args.c

# Custom libc implementations
LIBC_SRCS = ../my_libc/my_memset.c \
            ../my_libc/my_strcmp.c

# Test object files
TEST_OBJS = $(TEST_SRCS:.c=.o)
IMPL_OBJS = $(IMPL_SRCS:.c=.o)
LIBC_OBJS = $(LIBC_SRCS:.c=.o)

# Test executables
TEST_BINS = test_cli_parser

.PHONY: all clean test

all: $(TEST_BINS)

test: all
	@for test in $(TEST_BINS); do \
		echo "\nRunning $$test..."; \
		./$$test; \
	done

test_cli_parser: test_framework.o unit/test_cli_parser.o $(IMPL_OBJS) $(LIBC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TEST_OBJS) $(IMPL_OBJS) $(LIBC_OBJS) $(TEST_BINS)
