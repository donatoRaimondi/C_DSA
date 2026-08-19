CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g

.PHONY: all test sanitize clean

all: test_list

test_list: list.c list.h test_list.c
	$(CC) $(CFLAGS) list.c test_list.c -o $@

test: test_list
	./test_list

sanitize:
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer \
		list.c test_list.c -o test_list_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_list_sanitize

clean:
	rm -rf test_list test_list_sanitize test_list.dSYM test_list_sanitize.dSYM
