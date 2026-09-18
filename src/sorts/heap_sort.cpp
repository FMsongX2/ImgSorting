#include "recorder.h"

namespace {

void siftDown(Recorder& r, int lo, int root, int end) {
    for (;;) {
        int child = 2 * root + 1;
        if (child >= end) return;
        if (child + 1 < end && r.less(lo + child, lo + child + 1)) ++child;
        if (!r.less(lo + root, lo + child)) return;
        r.swap(lo + root, lo + child);
        root = child;
    }
}

}

void heapSortRange(Recorder& r, int lo, int hi) {
    const int n = hi - lo;
    for (int i = n / 2 - 1; i >= 0; --i) siftDown(r, lo, i, n);
    for (int end = n - 1; end > 0; --end) {
        r.swap(lo, lo + end);
        siftDown(r, lo, 0, end);
    }
}

void heapSort(Recorder& r) { heapSortRange(r, 0, r.size()); }
