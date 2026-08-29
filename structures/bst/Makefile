CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g
LIST_DIR := ../list
STACK_DIR := ../stack
QUEUE_DIR := ../queue
BTREE_DIR := ../b_tree

.PHONY: all test sanitize clean

all: test_bst

SOURCES := $(LIST_DIR)/list.c $(STACK_DIR)/stack.c $(QUEUE_DIR)/queue.c \
	$(BTREE_DIR)/b_tree.c bst.c test_bst.c
HEADERS := bst.h $(BTREE_DIR)/b_tree.h $(LIST_DIR)/list.h \
	$(STACK_DIR)/stack.h $(QUEUE_DIR)/queue.h

test_bst: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -I$(LIST_DIR) -I$(STACK_DIR) -I$(QUEUE_DIR) \
		-I$(BTREE_DIR) $(SOURCES) -o $@

test: test_bst
	./test_bst

sanitize:
	$(CC) $(CFLAGS) -I$(LIST_DIR) -I$(STACK_DIR) -I$(QUEUE_DIR) \
		-I$(BTREE_DIR) -fsanitize=address,undefined -fno-omit-frame-pointer \
		$(SOURCES) -o test_bst_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_bst_sanitize

clean:
	rm -rf test_bst test_bst_sanitize test_bst.dSYM test_bst_sanitize.dSYM
