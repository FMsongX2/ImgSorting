#include "recorder.h"

namespace {

void quickRange(Recorder& r, int lo, int hi) {
    if (lo >= hi) return;
    int p = lo;
    for (int j = lo; j < hi; ++j)
        if (r.less(j, hi)) {
            if (p != j) r.swap(p, j);
            ++p;
        }
    if (p != hi) r.swap(p, hi);
    quickRange(r, lo, p - 1);
    quickRange(r, p + 1, hi);
}

}

void quickSort(Recorder& r) { quickRange(r, 0, r.size() - 1); }
