#include "recorder.h"

#include <algorithm>

void radixSort(Recorder& r) {
    if (r.a.empty()) return;
    const int n = r.size();
    const int maxValue = *std::max_element(r.a.begin(), r.a.end());
    std::vector<int> out(n);
    for (int exp = 1; maxValue / exp > 0; exp *= 10) {
        int count[10] = {};
        for (int v : r.a) ++count[v / exp % 10];
        for (int d = 1; d < 10; ++d) count[d] += count[d - 1];
        for (int i = n - 1; i >= 0; --i) out[--count[r.a[i] / exp % 10]] = r.a[i];
        for (int i = 0; i < n; ++i) r.write(i, out[i]);
    }
}
