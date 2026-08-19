CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g
LIST_DIR := ../list

.PHONY: all test sanitize clean

all: test_stack

test_stack: stack.c stack.h test_stack.c $(LIST_DIR)/list.c $(LIST_DIR)/list.h
	$(CC) $(CFLAGS) -I$(LIST_DIR) $(LIST_DIR)/list.c stack.c test_stack.c \
		-o $@

test: test_stack
	./test_stack

sanitize:
	$(CC) $(CFLAGS) -I$(LIST_DIR) -fsanitize=address,undefined \
		-fno-omit-frame-pointer $(LIST_DIR)/list.c stack.c test_stack.c \
		-o test_stack_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_stack_sanitize

clean:
	rm -rf test_stack test_stack_sanitize test_stack.dSYM \
		test_stack_sanitize.dSYM
