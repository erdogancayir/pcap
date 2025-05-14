# Project Name and Compiler
NAME        := packet_sniffer
CC          := gcc

# Flags
CFLAGS      := -Wall -Wextra -Iinclude -Imy_libc -pthread -g3
DEBUG_FLAGS := -DDEBUG_MODE
LDFLAGS     := -lpcap

# Directories
SRC_DIR     := source
OBJ_DIR     := obj
LIBC_DIR    := my_libc

# Colors and styles
GREEN       := $(shell tput setaf 2)
YELLOW      := $(shell tput setaf 3)
BLUE        := $(shell tput setaf 4)
RED         := $(shell tput setaf 1)
BOLD        := $(shell tput bold)
RESET       := $(shell tput sgr0)

# Source and Object Files
SRCS        := $(shell find $(SRC_DIR) -name '*.c') $(shell find $(LIBC_DIR) -name '*.c')
OBJS        := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(NAME)

# Link the executable
$(NAME): $(OBJS)
	@echo "$(GREEN)🔗 Linking$(RESET) $(BOLD)$(NAME)$(RESET)..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "$(GREEN)[✔] Build successful!$(RESET)"

# Compile source files to object files
$(OBJ_DIR)/%.o: %.c
	@echo "$(BLUE)🛠 Compiling$(RESET) $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Debug target
debug: CFLAGS += $(DEBUG_FLAGS)
debug: re

# Clean object files
clean:
	@echo "$(YELLOW)🧹 Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

# Full clean
fclean: clean
	@echo "$(RED)🗑 Removing binary '$(NAME)'...$(RESET)"
	@rm -f $(NAME)

# Rebuild everything
re: fclean all

.PHONY: all debug clean fclean re


# Tests
TEST_DIR    := tests
TEST_OBJDIR := $(TEST_DIR)/obj
PERF_DIR    := $(TEST_DIR)/performance
PERF_OBJDIR := $(PERF_DIR)/obj

TEST_SRCS   := $(shell find $(TEST_DIR)/unit -name '*.c') $(TEST_DIR)/test_framework.c
TEST_OBJS   := $(patsubst %.c, $(TEST_OBJDIR)/%.o, $(TEST_SRCS))
TEST_BINS   := run_tests

PERF_SRCS   := $(shell find $(PERF_DIR) -name '*.c')
PERF_OBJS   := $(patsubst %.c, $(PERF_OBJDIR)/%.o, $(PERF_SRCS))
PERF_BINS   := run_perf_tests

# Performance test source files
PERF_SRC_FILES := $(SRC_DIR)/handlers/pcap/producer/packer_handler/handle_tcp_packet.c \
                  $(SRC_DIR)/handlers/pcap/producer/packer_handler/handle_udp_packet.c \
                  $(SRC_DIR)/packet_queue/packet_queue.c \
                  $(SRC_DIR)/tcp_tracker/tcp_tracker.c

PERF_SRC_OBJS := $(patsubst %.c, $(PERF_OBJDIR)/%.o, $(PERF_SRC_FILES))

# Compile test object files
$(TEST_OBJDIR)/%.o: %.c
	@echo "$(BLUE)🧪 Compiling test$(RESET) $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -I. -Iinclude -I$(TEST_DIR) -c $< -o $@

# Compile performance test object files
$(PERF_OBJDIR)/%.o: %.c
	@echo "$(BLUE)📊 Compiling performance test$(RESET) $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -I. -Iinclude -I$(TEST_DIR) -c $< -o $@

# Link test executable
$(TEST_BINS): $(TEST_OBJS)
	@echo "$(GREEN)🔗 Linking test binary$(RESET)..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Link performance test executable
$(PERF_BINS): $(PERF_OBJS) $(PERF_SRC_OBJS)
	@echo "$(GREEN)🔗 Linking performance test binary$(RESET)..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Run tests
test: $(TEST_BINS)
	@echo "$(BOLD)🔍 Running unit tests...$(RESET)"
	@./$(TEST_BINS)

# Run performance tests
perf: $(PERF_BINS)
	@echo "$(BOLD)📊 Running performance tests...$(RESET)"
	@sudo ./$(PERF_BINS)

# Clean tests
test_clean:
	@echo "$(YELLOW)🧹 Cleaning test objects...$(RESET)"
	@rm -rf $(TEST_OBJDIR) $(TEST_BINS) $(PERF_OBJDIR) $(PERF_BINS)