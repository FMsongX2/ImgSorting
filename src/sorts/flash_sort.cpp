#include "recorder.h"

#include <algorithm>
#include <vector>

void flashSort(Recorder& r) {
    const int n = r.size();
    if (n < 2) return;
    int minPos = 0, maxPos = 0;
    for (int i = 1; i < n; ++i) {
        if (r.less(i, minPos)) minPos = i;
        if (r.less(maxPos, i)) maxPos = i;
    }
    const int min = r.a[minPos], max = r.a[maxPos];
    if (min == max) return;
    const int classes = std::max(2, static_cast<int>(0.43 * n));
    auto classOf = [&](int v) { return static_cast<int>(static_cast<long long>(classes - 1) * (v - min) / (max - min)); };
    std::vector<int> end(classes, 0);
    for (int v : r.a) ++end[classOf(v)];
    for (int k = 1; k < classes; ++k) end[k] += end[k - 1];
    if (maxPos != 0) r.swap(maxPos, 0);
    for (int moved = 0, j = 0, k = classes - 1; moved < n - 1;) {
        while (j > end[k] - 1) k = classOf(r.a[++j]);
        for (int flash = r.a[j]; j != end[k];) {
            k = classOf(flash);
            const int slot = --end[k];
            const int held = r.a[slot];
            r.write(slot, flash);
            flash = held;
            ++moved;
        }
    }
    insertionSortRange(r, 0, n);
}
