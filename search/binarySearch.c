#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

size_t binarySearch(const int array[], int key, size_t size);

int main(int argc, char *argv[]) {
  int array[100];

  for (size_t x = 0; x < 100; x++) {
    array[x] = 2 * x;
  }

  printf("Enter integer search key:");
  int searchKey;
  scanf("%d", &searchKey);

  size_t index = binarySearch(array, searchKey, 100);

  if (index != -1)
    printf("Found value in element %lu\n", index);
  else
    puts("Value not found");

  return EXIT_SUCCESS;
}

size_t binarySearch(const int array[], int key, size_t size) {
  size_t low = 0;
  size_t high = size - 1;

  while (low <= high) {
    size_t middle = low + (high - low) / 2;

    if (key == array[middle]) {
      return middle;
    } else if (key < array[middle]) {
      high = middle - 1;
    } else {
      low = middle + 1;
    }
  }

  return -1;
}
