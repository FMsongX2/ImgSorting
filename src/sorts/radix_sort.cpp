#include "recorder.h"

#include <algorithm>

void radixSort(Recorder& r) {
    if (r.a.empty()) return;
    const int maxValue = *std::max_element(r.a.begin(), r.a.end());
    for (int exp = 1; maxValue / exp > 0; exp *= 10) {
        const std::vector<int> in = r.a;
        int start[11] = {};
        for (int v : in) ++start[v / exp % 10 + 1];
        for (int d = 0; d < 10; ++d) start[d + 1] += start[d];
        for (int v : in) r.write(start[v / exp % 10]++, v);
    }
}
