#include <assert.h>
#include <stdio.h>
#include "insertion_sort.h"

static void assert_array_equals(int actual[], int expected[], int n) {
    for (int i = 0; i < n; i++) {
        assert(actual[i] == expected[i]);
    }
}

static void test_unsorted_array(void) {
    int arr[] = {12, 11, 13, 5, 6};
    int expected[] = {5, 6, 11, 12, 13};
    int n = sizeof(arr) / sizeof(arr[0]);

    insertionSort(arr, n);

    assert_array_equals(arr, expected, n);
}

static void test_already_sorted_array(void) {
    int arr[] = {1, 2, 3, 4, 5};
    int expected[] = {1, 2, 3, 4, 5};
    int n = sizeof(arr) / sizeof(arr[0]);

    insertionSort(arr, n);

    assert_array_equals(arr, expected, n);
}

static void test_reverse_sorted_array(void) {
    int arr[] = {5, 4, 3, 2, 1};
    int expected[] = {1, 2, 3, 4, 5};
    int n = sizeof(arr) / sizeof(arr[0]);

    insertionSort(arr, n);

    assert_array_equals(arr, expected, n);
}

static void test_array_with_duplicates(void) {
    int arr[] = {4, 2, 4, 1, 2};
    int expected[] = {1, 2, 2, 4, 4};
    int n = sizeof(arr) / sizeof(arr[0]);

    insertionSort(arr, n);

    assert_array_equals(arr, expected, n);
}

static void test_array_with_negative_numbers(void) {
    int arr[] = {-3, 10, 0, -1, 5};
    int expected[] = {-3, -1, 0, 5, 10};
    int n = sizeof(arr) / sizeof(arr[0]);

    insertionSort(arr, n);

    assert_array_equals(arr, expected, n);
}

static void test_empty_array(void) {
    int arr[] = {999};   // dummy value, should not be touched
    int expected[] = {999};

    insertionSort(arr, 0);

    assert_array_equals(arr, expected, 1);
}

static void test_null_array_with_zero_size(void) {
    insertionSort(NULL, 0);
}

static void test_two_elements_unsorted(void) {
    int arr[] = {2, 1};
    int expected[] = {1, 2};

    insertionSort(arr, 2);

    assert_array_equals(arr, expected, 2);
}

static void test_two_elements_sorted(void) {
    int arr[] = {1, 2};
    int expected[] = {1, 2};

    insertionSort(arr, 2);

    assert_array_equals(arr, expected, 2);
}

static void test_all_equal_elements(void) {
    int arr[] = {7, 7, 7, 7, 7};
    int expected[] = {7, 7, 7, 7, 7};

    insertionSort(arr, 5);

    assert_array_equals(arr, expected, 5);
}

static void test_int_min_max(void) {
    int arr[] = {INT_MAX, 0, INT_MIN};
    int expected[] = {INT_MIN, 0, INT_MAX};

    insertionSort(arr, 3);

    assert_array_equals(arr, expected, 3);
}

static void test_single_element_array(void) {
    int arr[] = {42};
    int expected[] = {42};
    int n = sizeof(arr) / sizeof(arr[0]);

    insertionSort(arr, n);

    assert_array_equals(arr, expected, n);
}

int main(void) {
    test_empty_array();
    test_null_array_with_zero_size();
    test_two_elements_unsorted();
    test_two_elements_sorted();
    test_all_equal_elements();
    test_int_min_max();

    test_unsorted_array();
    test_already_sorted_array();
    test_reverse_sorted_array();
    test_array_with_duplicates();
    test_array_with_negative_numbers();
    test_single_element_array();

    printf("All insertion sort tests passed.\n");
    return 0;
}