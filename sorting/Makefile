CC = cc
CFLAGS = -Wall -Wextra -Werror -Iinclude
BUILD_DIR = build

TEST ?= insertion_sort

SRC = src/$(TEST).c
TEST_SRC = tests/test_$(TEST).c
TARGET = $(BUILD_DIR)/test_$(TEST)

all: $(TARGET)

$(TARGET): $(SRC) $(TEST_SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) $(TEST_SRC) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

re: clean all

.PHONY: all test clean re