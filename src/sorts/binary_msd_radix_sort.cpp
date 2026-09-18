#include "recorder.h"

#include <algorithm>
#include <bit>

namespace {

void radixExchange(Recorder& r, int lo, int hi, int bit) {
    if (hi - lo < 2 || bit < 0) return;
    int i = lo, j = hi - 1;
    for (;;) {
        while (i <= j && !((r.a[i] >> bit) & 1)) ++i;
        while (i <= j && ((r.a[j] >> bit) & 1)) --j;
        if (i >= j) break;
        r.swap(i, j);
    }
    radixExchange(r, lo, i, bit - 1);
    radixExchange(r, i, hi, bit - 1);
}

}

void binaryMsdRadixSort(Recorder& r) {
    if (r.a.empty()) return;
    const unsigned max = static_cast<unsigned>(*std::max_element(r.a.begin(), r.a.end()));
    radixExchange(r, 0, r.size(), static_cast<int>(std::bit_width(max)) - 1);
}
