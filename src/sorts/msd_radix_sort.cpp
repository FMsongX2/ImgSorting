#include "recorder.h"

#include <algorithm>
#include <vector>

namespace {

void msd(Recorder& r, int lo, int hi, int exp) {
    if (hi - lo < 2 || exp == 0) return;
    int start[11] = {};
    for (int i = lo; i < hi; ++i) ++start[r.a[i] / exp % 10 + 1];
    for (int d = 0; d < 10; ++d) start[d + 1] += start[d];
    std::vector<int> out(hi - lo);
    int fill[10];
    std::copy(start, start + 10, fill);
    for (int i = lo; i < hi; ++i) out[fill[r.a[i] / exp % 10]++] = r.a[i];
    for (int i = lo; i < hi; ++i) r.write(i, out[i - lo]);
    for (int d = 0; d < 10; ++d) msd(r, lo + start[d], lo + start[d + 1], exp / 10);
}

}

void msdRadixSort(Recorder& r) {
    if (r.a.empty()) return;
    const int max = *std::max_element(r.a.begin(), r.a.end());
    int exp = 1;
    while (max / exp >= 10) exp *= 10;
    msd(r, 0, r.size(), exp);
}
