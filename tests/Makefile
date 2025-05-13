CC = gcc
CFLAGS = -Wall -Wextra -g -I../include -I.
LDFLAGS = -lpcap -pthread

# Test source files
TEST_SRCS = unit/test_packet_parsing.c \
            unit/test_thread_safety.c \
            integration/test_pcap_processing.c

# Test object files
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_BINS = $(TEST_SRCS:.c=)

# Framework files
FRAMEWORK_SRC = test_framework.c
FRAMEWORK_OBJ = $(FRAMEWORK_SRC:.c=.o)

# Implementation files
IMPL_SRCS = ../source/handlers/pcap/packet_queue/packet_queue.c \
            ../source/utils/thread_safe_counter.c
IMPL_OBJS = $(IMPL_SRCS:.c=.o)

.PHONY: all clean test test-unit test-integration test-packet-parsing test-thread-safety test-pcap-processing test-fixtures

# Build all test binaries
all: $(TEST_BINS)

# Run all tests
test: all
	@for test in $(TEST_BINS); do \
		echo "Running $$test..."; \
		./$$test; \
	done

# Run unit tests
test-unit: unit/test_packet_parsing unit/test_thread_safety
	@for test in $^; do \
		echo "Running $$test..."; \
		./$$test; \
	done

# Run integration tests
test-integration: integration/test_pcap_processing
	@for test in $^; do \
		echo "Running $$test..."; \
		./$$test; \
	done

# Run individual tests
test-packet-parsing: unit/test_packet_parsing
	@echo "Running $<..."
	@./$<

test-thread-safety: unit/test_thread_safety
	@echo "Running $<..."
	@./$<

test-pcap-processing: integration/test_pcap_processing
	@echo "Running $<..."
	@./$<

# Generate test fixtures
test-fixtures: fixtures/generate_test_pcap
	@echo "Generating test fixtures..."
	@mkdir -p fixtures
	@./fixtures/generate_test_pcap

# Compile test binaries
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile implementation files
../source/handlers/pcap/packet_queue/%.o: ../source/handlers/pcap/packet_queue/%.c
	$(CC) $(CFLAGS) -c $< -o $@

../source/utils/thread_safe_counter.o: ../source/utils/thread_safe_counter.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link test binaries with framework and implementation
$(TEST_BINS): %: %.o $(FRAMEWORK_OBJ) $(IMPL_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile framework
$(FRAMEWORK_OBJ): $(FRAMEWORK_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TEST_OBJS) $(TEST_BINS) $(FRAMEWORK_OBJ) $(IMPL_OBJS) fixtures/*.pcap 