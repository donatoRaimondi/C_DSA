CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g
LIST_DIR := ../list
STACK_DIR := ../stack
QUEUE_DIR := ../queue

.PHONY: all test sanitize clean

all: test_b_tree

SOURCES := $(LIST_DIR)/list.c $(STACK_DIR)/stack.c $(QUEUE_DIR)/queue.c \
	 b_tree.c test_b_tree.c
HEADERS := b_tree.h $(LIST_DIR)/list.h $(STACK_DIR)/stack.h $(QUEUE_DIR)/queue.h

test_b_tree: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -I$(LIST_DIR) -I$(STACK_DIR) -I$(QUEUE_DIR) \
		$(SOURCES) -o $@

test: test_b_tree
	./test_b_tree

sanitize:
	$(CC) $(CFLAGS) -I$(LIST_DIR) -I$(STACK_DIR) -I$(QUEUE_DIR) \
		-fsanitize=address,undefined -fno-omit-frame-pointer $(SOURCES) \
		-o test_b_tree_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_b_tree_sanitize

clean:
	rm -rf test_b_tree test_b_tree_sanitize test_b_tree.dSYM \
		test_b_tree_sanitize.dSYM
