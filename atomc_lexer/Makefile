# =============================================================================
#  Makefile for AtomC Lexical Analyzer
#  Compilation Techniques Laboratory
# =============================================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g
TARGET  = lexer
SRC     = lexer.c

# Default target: build the lexer
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Run all tests
test: $(TARGET)
	@for i in 0 1 2 3 4 5 6 7 8 9; do \
		echo "================ TEST $$i.c ================"; \
		./$(TARGET) tests/$$i.c; \
		echo ""; \
	done

# Run a single test:  make run T=5
run: $(TARGET)
	./$(TARGET) tests/$(T).c

# Clean build artifacts
clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all test run clean
