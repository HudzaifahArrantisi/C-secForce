# CyberForce Build System
CC = gcc

# Detect OS
ifeq ($(OS),Windows_NT)
    CFLAGS = -Wall -Wextra -O2 -D_WIN32 -DCURL_STATICLIB
    LIBS = -lcurl -lssh -lssl -lcrypto -lws2_32 -lm -lpthread
    TARGET = cyberforce.exe
    RM = del /Q
    MKDIR = if not exist
else
    CFLAGS = -Wall -Wextra -O2 -pthread -D_GNU_SOURCE
    LIBS = -lcurl -lssh -lssl -lcrypto -lm -lpthread
    TARGET = cyberforce
    RM = rm -f
    MKDIR = mkdir -p
endif

VERSION = 2.0.0

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
MODULES_DIR = $(SRC_DIR)/modules
UTILS_DIR = $(SRC_DIR)/utils
INCLUDE_DIR = include
DATA_DIR = data
EXAMPLES_DIR = examples

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(CORE_DIR)/attack_engine.c \
       $(CORE_DIR)/thread_manager.c \
       $(CORE_DIR)/rate_limiter.c \
       $(CORE_DIR)/protocol_handlers.c \
       $(MODULES_DIR)/http_brute.c \
       $(MODULES_DIR)/ftp_brute.c \
       $(MODULES_DIR)/ssh_brute.c \
       $(MODULES_DIR)/mysql_brute.c \
       $(UTILS_DIR)/wordlist_gen.c \
       $(UTILS_DIR)/pattern_matcher.c \
       $(UTILS_DIR)/proxy_rotator.c \
       $(UTILS_DIR)/logger.c \
       $(UTILS_DIR)/crypto_helper.c

# Object files
OBJS = $(SRCS:.c=.o)

# Colors for output
GREEN = \033[0;32m
YELLOW = \033[1;33m
RED = \033[0;31m
BLUE = \033[0;34m
NC = \033[0m

# Default target
all: $(TARGET)

# Link target
$(TARGET): $(OBJS)
	@echo "$(GREEN)[+] Linking $(TARGET)...$(NC)"
	@$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo "$(GREEN)[+] Build successful!$(NC)"
	@echo "$(YELLOW)[*] Version: $(VERSION)$(NC)"

# Compile source files
%.o: %.c
	@echo "$(BLUE)[*] Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) -I$(SRC_DIR)/$(INCLUDE_DIR) -c $< -o $@

# Clean build files
clean:
	@echo "$(RED)[-] Cleaning build files...$(NC)"
ifeq ($(OS),Windows_NT)
	-@del /Q $(subst /,\,$(OBJS)) $(TARGET) *.log *.checkpoint 2>nul
else
	@rm -f $(OBJS) $(TARGET) *.log *.checkpoint
	@rm -rf $(DATA_DIR)/output/*
endif

# Install to system
install:
	@echo "$(GREEN)[+] Installing $(TARGET)...$(NC)"
	@cp $(TARGET) /usr/local/bin/
	@mkdir -p /etc/cyberforce
	@cp -r $(DATA_DIR)/wordlists /etc/cyberforce/
	@chmod +x /usr/local/bin/$(TARGET)
	@echo "$(GREEN)[+] Installation complete$(NC)"

# Uninstall from system
uninstall:
	@echo "$(RED)[-] Uninstalling $(TARGET)...$(NC)"
	@rm -f /usr/local/bin/$(TARGET)
	@rm -rf /etc/cyberforce
	@echo "$(GREEN)[+] Uninstallation complete$(NC)"

# Debug build
debug: CFLAGS += -g -DDEBUG -fsanitize=address
debug: clean $(TARGET)
	@echo "$(YELLOW)[*] Debug build created$(NC)"

# Release build
release: CFLAGS += -O3 -flto -DNDEBUG
release: clean $(TARGET)
	@echo "$(GREEN)[+] Release build created$(NC)"

# Run tests
test: $(TARGET)
	@echo "$(YELLOW)[*] Running tests...$(NC)"
	@cd tests && ./run_tests.sh

# Install dependencies (Ubuntu/Debian)
deps:
	@echo "$(YELLOW)[*] Installing dependencies...$(NC)"
	@sudo apt-get update
	@sudo apt-get install -y libcurl4-openssl-dev libssl-dev libssh-dev
	@sudo apt-get install -y build-essential git curl wget
	@echo "$(GREEN)[+] Dependencies installed$(NC)"

# Create data directories
data-dirs:
	@mkdir -p $(DATA_DIR)/wordlists
	@mkdir -p $(DATA_DIR)/output
	@mkdir -p $(DATA_DIR)/logs
	@echo "$(GREEN)[+] Data directories created$(NC)"

# Generate documentation
docs:
	@echo "$(YELLOW)[*] Generating documentation...$(NC)"
	@mkdir -p docs
	@doxygen Doxyfile 2>/dev/null || echo "$(RED)[!] Doxygen not found$(NC)"
	@echo "$(GREEN)[+] Documentation generated in docs/$(NC)"

# Format code
format:
	@echo "$(YELLOW)[*] Formatting code...$(NC)"
	@find $(SRC_DIR) -name "*.c" -exec clang-format -i {} \;
	@find $(INCLUDE_DIR) -name "*.h" -exec clang-format -i {} \;
	@echo "$(GREEN)[+] Code formatted$(NC)"

# Static analysis
analyze:
	@echo "$(YELLOW)[*] Running static analysis...$(NC)"
	@scan-build make clean all 2>/dev/null || echo "$(RED)[!] scan-build not found$(NC)"

# Package for distribution
dist: clean
	@echo "$(YELLOW)[*] Creating distribution package...$(NC)"
	@mkdir -p dist/cyberforce-$(VERSION)
	@cp -r $(SRC_DIR) $(INCLUDE_DIR) $(DATA_DIR) $(EXAMPLES_DIR) Makefile *.md LICENSE dist/cyberforce-$(VERSION)/
	@tar -czf cyberforce-$(VERSION).tar.gz -C dist cyberforce-$(VERSION)
	@rm -rf dist
	@echo "$(GREEN)[+] Distribution package created: cyberforce-$(VERSION).tar.gz$(NC)"

# Help
help:
	@echo "$(YELLOW)CyberForce Build System$(NC)"
	@echo ""
	@echo "Available targets:"
	@echo "  $(GREEN)all$(NC)       - Build cyberforce (default)"
	@echo "  $(GREEN)clean$(NC)     - Remove build files"
	@echo "  $(GREEN)install$(NC)   - Install to system"
	@echo "  $(GREEN)uninstall$(NC) - Remove from system"
	@echo "  $(GREEN)debug$(NC)     - Build with debug symbols"
	@echo "  $(GREEN)release$(NC)   - Build optimized release"
	@echo "  $(GREEN)test$(NC)      - Run test suite"
	@echo "  $(GREEN)deps$(NC)      - Install dependencies"
	@echo "  $(GREEN)data-dirs$(NC) - Create data directories"
	@echo "  $(GREEN)docs$(NC)      - Generate documentation"
	@echo "  $(GREEN)format$(NC)    - Format source code"
	@echo "  $(GREEN)analyze$(NC)   - Run static analysis"
	@echo "  $(GREEN)dist$(NC)      - Create distribution package"
	@echo "  $(GREEN)help$(NC)      - Show this help"

.PHONY: all clean install uninstall debug release test deps data-dirs docs format analyze dist help