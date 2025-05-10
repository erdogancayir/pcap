# Project Name and Compiler
NAME        := packet_sniffer
CC          := gcc

# Flags
CFLAGS      := -Wall -Wextra -Iinclude -Imy_libc -pthread -DDEBUG_MODE -g3
DEBUG_FLAGS := -g3 -DDEBUG_MODE
LDFLAGS     := -lpcap

# Directories
SRC_DIR     := source
OBJ_DIR     := obj
LIBC_DIR    := my_libc

# Source files
SRCS        := $(shell find $(SRC_DIR) -name '*.c') $(shell find $(LIBC_DIR) -name '*.c')
OBJS        := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(NAME)

# Link the executable
$(NAME): $(OBJS)
	@echo "🔗 Linking $(NAME)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source and libc files into object files
$(OBJ_DIR)/%.o: %.c
	@echo "🛠 Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Debug target
debug: CFLAGS += $(DEBUG_FLAGS)
debug: re

# Clean object files and binary
clean:
	@echo "🧹 Cleaning..."
	rm -rf $(OBJ_DIR)

fclean: clean
	@echo "🗑 Removing binary..."
	rm -f $(NAME)

# Rebuild everything
re: fclean all

.PHONY: all debug clean fclean re