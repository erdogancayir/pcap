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