#include "recorder.h"

#include <algorithm>
#include <cmath>
#include <vector>

void bucketSort(Recorder& r) {
    const int n = r.size();
    if (n == 0) return;
    const auto [lo, hi] = std::minmax_element(r.a.begin(), r.a.end());
    const int min = *lo, range = *hi - *lo + 1;
    const int count = std::max(1, static_cast<int>(std::sqrt(n)));
    std::vector<std::vector<int>> buckets(count);
    for (int v : r.a) buckets[static_cast<long long>(v - min) * count / range].push_back(v);
    std::vector<int> bounds{0};
    int k = 0;
    for (const std::vector<int>& bucket : buckets) {
        for (int v : bucket) r.write(k++, v);
        bounds.push_back(k);
    }
    for (int b = 0; b < count; ++b) insertionSortRange(r, bounds[b], bounds[b + 1]);
}
