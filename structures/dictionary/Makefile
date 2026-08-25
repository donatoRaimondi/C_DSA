CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g

.PHONY: all test sanitize clean

all: test_dictionary

test_dictionary: dictionary.c dictionary.h test_dictionary.c
	$(CC) $(CFLAGS) dictionary.c test_dictionary.c -o $@

test: test_dictionary
	./test_dictionary

sanitize:
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer \
		dictionary.c test_dictionary.c -o test_dictionary_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./test_dictionary_sanitize

clean:
	rm -rf test_dictionary test_dictionary_sanitize test_dictionary.dSYM \
		test_dictionary_sanitize.dSYM
