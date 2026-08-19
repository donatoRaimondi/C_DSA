CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g
LIST_DIR := ../list

.PHONY: all test sanitize clean

all: test_ordered_list

test_ordered_list: ordered_list.c ordered_list.h test_ordered_list.c \
		$(LIST_DIR)/list.c $(LIST_DIR)/list.h
	$(CC) $(CFLAGS) -I$(LIST_DIR) $(LIST_DIR)/list.c ordered_list.c \
		test_ordered_list.c -o $@

test: test_ordered_list
	./test_ordered_list

sanitize:
	$(CC) $(CFLAGS) -I$(LIST_DIR) -fsanitize=address,undefined \
		-fno-omit-frame-pointer $(LIST_DIR)/list.c ordered_list.c \
		test_ordered_list.c -o test_ordered_list_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_ordered_list_sanitize

clean:
	rm -rf test_ordered_list test_ordered_list_sanitize \
		test_ordered_list.dSYM test_ordered_list_sanitize.dSYM
