#include "recorder.h"

namespace {

void slow(Recorder& r, int i, int j) {
    if (i >= j) return;
    const int m = (i + j) / 2;
    slow(r, i, m);
    slow(r, m + 1, j);
    if (r.less(j, m)) r.swap(j, m);
    slow(r, i, j - 1);
}

}

void slowSort(Recorder& r) { slow(r, 0, r.size() - 1); }
