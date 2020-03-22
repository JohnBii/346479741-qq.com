#include <stdio.h>
#include <stdlib.h>
#define N 100
static A[N];

int fun(int left, int right) {
    if ((right - left) == 1)
        return 0;
    int i = left + (right - left) / 2;
    if (A[i] == i)
        return i;
    else if (A[i] > i)
        right = i;
    else
        left = i;
    return fun(left, right);
}

void main() {
    int left = 0, right = N - 1;
    int result = 0;
    if (A[left] == left)
        result = left;
    else if (A[right] == right)
        result = right;
    else if (A[right] < right)
        result = 0;
    else if (A[left] > left)
        result = 0;
    else
        result = fun(left, right);
    if (result == 0)
        printf("no such number in the Array!\n");
    else
        printf("in the Array, A[%d]=%d\n", result, result);
}