CC = gcc
CFLAGS = -Wall -Wextra -g -I../include
LDFLAGS = -lpcap -pthread

TEST_SRCS = test_framework.c \
            unit/test_packet_parsing.c \
            unit/test_thread_safety.c \
            integration/test_pcap_processing.c \
            fixtures/generate_test_pcap.c

TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY: all clean test

all: $(TEST_BINS)

test: all
	@echo "Generating test fixtures..."
	@./fixtures/generate_test_pcap
	@echo "\nRunning unit tests..."
	@./unit/test_packet_parsing
	@./unit/test_thread_safety
	@echo "\nRunning integration tests..."
	@./integration/test_pcap_processing

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o test_framework.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TEST_OBJS) $(TEST_BINS) fixtures/*.pcap 