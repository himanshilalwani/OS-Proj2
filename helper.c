#include <stdio.h>
#include "helper.h"

// This function takes in two sorted subarrays and merges them into one sorted array.
void merge(int arr[], int left[], int leftSize, int right[], int rightSize)
{
    int i = 0, j = 0, k = 0;
    // Compare elements of the two subarrays and merge them into a single sorted array.
    while (i < leftSize && j < rightSize)
    {
        if (left[i] <= right[j])
        {
            arr[k] = left[i];
            i++;
        }
        else
        {
            arr[k] = right[j];
            j++;
        }
        k++;
    }

    // Copy any remaining elements from the left subarray into the merged array.
    while (i < leftSize)
    {
        arr[k] = left[i];
        i++;
        k++;
    }

    // Copy any remaining elements from the right subarray into the merged array.
    while (j < rightSize)
    {
        arr[k] = right[j];
        j++;
        k++;
    }
}

// This function implements the merge sort algorithm.
void mergeSort(int arr[], int n)
{
    if (n < 2)
    {
        return;
    }

    int mid = n / 2;
    int left[mid], right[n - mid];

    // Divide the array into two halves and copy the values into separate left and right arrays.
    for (int i = 0; i < mid; i++)
    {
        left[i] = arr[i];
    }
    for (int i = mid; i < n; i++)
    {
        right[i - mid] = arr[i];
    }

    // Recursively sort the left and right subarrays.
    mergeSort(left, mid);
    mergeSort(right, n - mid);

    // Merge the sorted left and right subarrays into a single sorted array.
    merge(arr, left, mid, right, n - mid);
}
