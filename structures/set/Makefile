CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g

.PHONY: all test sanitize clean

all: test_set

test_set: set.c set.h test_set.c
	$(CC) $(CFLAGS) set.c test_set.c -o $@

test: test_set
	./test_set

sanitize:
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer \
		set.c test_set.c -o test_set_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_set_sanitize

clean:
	rm -rf test_set test_set_sanitize test_set.dSYM test_set_sanitize.dSYM
