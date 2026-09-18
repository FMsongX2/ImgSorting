#include "recorder.h"

namespace {

void stoogeRange(Recorder& r, int lo, int hi) {
    if (r.less(hi, lo)) r.swap(lo, hi);
    if (hi - lo + 1 < 3) return;
    const int third = (hi - lo + 1) / 3;
    stoogeRange(r, lo, hi - third);
    stoogeRange(r, lo + third, hi);
    stoogeRange(r, lo, hi - third);
}

}

void stoogeSort(Recorder& r) {
    if (r.size() > 1) stoogeRange(r, 0, r.size() - 1);
}
