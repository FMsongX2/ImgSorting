#include "recorder.h"

void mergeAdjacent(Recorder& r, int lo, int mid, int hi) {
    const std::vector<int> left(r.a.begin() + lo, r.a.begin() + mid);
    const std::vector<int> right(r.a.begin() + mid, r.a.begin() + hi);
    int i = 0, j = 0, k = lo;
    const int nl = mid - lo, nr = hi - mid;
    while (i < nl && j < nr) {
        r.compare(k, mid + j);
        r.write(k++, right[j] < left[i] ? right[j++] : left[i++]);
    }
    while (i < nl) r.write(k++, left[i++]);
    while (j < nr) r.write(k++, right[j++]);
}

namespace {

void mergeRange(Recorder& r, int lo, int hi) {
    if (hi - lo < 2) return;
    const int mid = (lo + hi) / 2;
    mergeRange(r, lo, mid);
    mergeRange(r, mid, hi);
    mergeAdjacent(r, lo, mid, hi);
}

}

void mergeSort(Recorder& r) { mergeRange(r, 0, r.size()); }
