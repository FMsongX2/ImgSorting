#include "recorder.h"

#include <algorithm>
#include <vector>

void pigeonholeSort(Recorder& r) {
    if (r.a.empty()) return;
    const auto [lo, hi] = std::minmax_element(r.a.begin(), r.a.end());
    const int min = *lo;
    std::vector<std::vector<int>> holes(*hi - min + 1);
    for (int v : r.a) holes[v - min].push_back(v);
    int k = 0;
    for (const std::vector<int>& hole : holes)
        for (int v : hole) r.write(k++, v);
}
