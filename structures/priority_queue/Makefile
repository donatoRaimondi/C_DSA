CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g

.PHONY: all test sanitize clean

all: test_priority_queue

test_priority_queue: priority_queue.c priority_queue.h test_priority_queue.c
	$(CC) $(CFLAGS) priority_queue.c test_priority_queue.c -o $@

test: test_priority_queue
	./test_priority_queue

sanitize:
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer \
		priority_queue.c test_priority_queue.c -o test_priority_queue_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_priority_queue_sanitize

clean:
	rm -rf test_priority_queue test_priority_queue_sanitize \
		test_priority_queue.dSYM test_priority_queue_sanitize.dSYM
