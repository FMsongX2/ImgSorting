#include "recorder.h"

void insertionSortRange(Recorder& r, int lo, int hi) {
    for (int i = lo + 1; i < hi; ++i)
        for (int j = i; j > lo && r.less(j, j - 1); --j) r.swap(j, j - 1);
}

void insertionSort(Recorder& r) { insertionSortRange(r, 0, r.size()); }
