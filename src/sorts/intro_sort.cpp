#include "recorder.h"

namespace {

constexpr int kSmallRange = 16;

void introLoop(Recorder& r, int lo, int hi, int depth) {
    while (hi - lo > kSmallRange) {
        if (depth-- == 0) {
            heapSortRange(r, lo, hi);
            return;
        }
        const int mid = lo + (hi - lo) / 2, last = hi - 1;
        if (r.less(mid, lo)) r.swap(mid, lo);
        if (r.less(last, lo)) r.swap(last, lo);
        if (r.less(last, mid)) r.swap(last, mid);
        r.swap(mid, last);
        int p = lo;
        for (int j = lo; j < last; ++j)
            if (r.less(j, last)) {
                if (p != j) r.swap(p, j);
                ++p;
            }
        if (p != last) r.swap(p, last);
        introLoop(r, p + 1, hi, depth);
        hi = p;
    }
}

}

void introSort(Recorder& r) {
    const int n = r.size();
    int depth = 0;
    for (int m = n; m > 1; m >>= 1) depth += 2;
    introLoop(r, 0, n, depth);
    insertionSortRange(r, 0, n);
}
