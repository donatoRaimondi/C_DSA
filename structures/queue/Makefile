CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g
LIST_DIR := ../list

.PHONY: all test sanitize clean

all: test_queue

test_queue: queue.c queue.h test_queue.c $(LIST_DIR)/list.c $(LIST_DIR)/list.h
	$(CC) $(CFLAGS) -I$(LIST_DIR) $(LIST_DIR)/list.c queue.c test_queue.c \
		-o $@

test: test_queue
	./test_queue

sanitize:
	$(CC) $(CFLAGS) -I$(LIST_DIR) -fsanitize=address,undefined \
		-fno-omit-frame-pointer $(LIST_DIR)/list.c queue.c test_queue.c \
		-o test_queue_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_queue_sanitize

clean:
	rm -rf test_queue test_queue_sanitize test_queue.dSYM \
		test_queue_sanitize.dSYM
