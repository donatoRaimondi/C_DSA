#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

size_t linearSearch(const int array[], int key, size_t size);

int main(int argc, char *argv[]) {
  int array[100];

  for (size_t x = 0; x < 100; x++) {
    array[x] = 2 * x;
  }

  printf("Enter integer search key:");
  int searchKey;
  scanf("%d", &searchKey);

  size_t index = linearSearch(array, searchKey, 100);

  if (index != -1)
    printf("Found value in element %lu\n", index);
  else
    puts("Value not found");

  return EXIT_SUCCESS;
}

size_t linearSearch(const int array[], int key, size_t size) {
  // ciclo attraverso l'array
  for (size_t i = 0; i < size; i++) {
    if (array[i] == key) {
      return i;
    }
  }

  return -1;
}
