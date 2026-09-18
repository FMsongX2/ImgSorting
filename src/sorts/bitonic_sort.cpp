#include "recorder.h"

namespace {

int powerOfTwoBelow(int n) {
    int k = 1;
    while (k < n) k <<= 1;
    return k >> 1;
}

void compareSwap(Recorder& r, int i, int j, bool ascending) {
    if (ascending == r.less(j, i)) r.swap(i, j);
}

void bitonicMerge(Recorder& r, int lo, int n, bool ascending) {
    if (n < 2) return;
    const int m = powerOfTwoBelow(n);
    for (int i = lo; i < lo + n - m; ++i) compareSwap(r, i, i + m, ascending);
    bitonicMerge(r, lo, m, ascending);
    bitonicMerge(r, lo + m, n - m, ascending);
}

void bitonic(Recorder& r, int lo, int n, bool ascending) {
    if (n < 2) return;
    const int half = n / 2;
    bitonic(r, lo, half, !ascending);
    bitonic(r, lo + half, n - half, ascending);
    bitonicMerge(r, lo, n, ascending);
}

}

void bitonicSort(Recorder& r) { bitonic(r, 0, r.size(), true); }
