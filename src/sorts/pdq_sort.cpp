#include "recorder.h"

namespace {

constexpr int kInsertionThreshold = 24;
constexpr int kPartialInsertionLimit = 8;

void sort2(Recorder& r, int a, int b) {
    if (r.less(b, a)) r.swap(a, b);
}

bool partialInsertionSort(Recorder& r, int lo, int hi) {
    int moved = 0;
    for (int i = lo + 1; i < hi; ++i) {
        int j = i;
        for (; j > lo && r.less(j, j - 1); --j) r.swap(j, j - 1);
        moved += i - j;
        if (moved > kPartialInsertionLimit) return false;
    }
    return true;
}

int partitionRight(Recorder& r, int lo, int hi, bool& alreadyPartitioned) {
    int first = lo, last = hi;
    while (r.less(++first, lo)) {}
    if (first - 1 == lo)
        while (first < last && !r.less(--last, lo)) {}
    else
        while (!r.less(--last, lo)) {}
    alreadyPartitioned = first >= last;
    while (first < last) {
        r.swap(first, last);
        while (r.less(++first, lo)) {}
        while (!r.less(--last, lo)) {}
    }
    const int pivot = first - 1;
    if (pivot != lo) r.swap(lo, pivot);
    return pivot;
}

void pdqLoop(Recorder& r, int lo, int hi, int badAllowed) {
    for (;;) {
        const int size = hi - lo;
        if (size < kInsertionThreshold) {
            insertionSortRange(r, lo, hi);
            return;
        }
        const int mid = lo + size / 2;
        sort2(r, mid, lo);
        sort2(r, lo, hi - 1);
        sort2(r, mid, lo);
        bool alreadyPartitioned = false;
        const int pivot = partitionRight(r, lo, hi, alreadyPartitioned);
        const int leftSize = pivot - lo, rightSize = hi - (pivot + 1);
        if (leftSize < size / 8 || rightSize < size / 8) {
            if (--badAllowed == 0) {
                heapSortRange(r, lo, hi);
                return;
            }
            if (leftSize >= kInsertionThreshold) {
                r.swap(lo, lo + leftSize / 4);
                r.swap(pivot - 1, pivot - leftSize / 4);
            }
            if (rightSize >= kInsertionThreshold) {
                r.swap(pivot + 1, pivot + 1 + rightSize / 4);
                r.swap(hi - 1, hi - rightSize / 4);
            }
        } else if (alreadyPartitioned && partialInsertionSort(r, lo, pivot) && partialInsertionSort(r, pivot + 1, hi)) {
            return;
        }
        pdqLoop(r, lo, pivot, badAllowed);
        lo = pivot + 1;
    }
}

}

void pdqSort(Recorder& r) {
    int badAllowed = 1;
    for (int m = r.size(); m > 1; m >>= 1) ++badAllowed;
    pdqLoop(r, 0, r.size(), badAllowed);
}
