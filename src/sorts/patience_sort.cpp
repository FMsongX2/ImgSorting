#include "recorder.h"

#include <vector>

void patienceSort(Recorder& r) {
    const int n = r.size();
    const std::vector<int> v = r.a;
    std::vector<std::vector<int>> piles;
    for (int i = 0; i < n; ++i) {
        int lo = 0, hi = static_cast<int>(piles.size());
        while (lo < hi) {
            const int mid = (lo + hi) / 2;
            if (r.less(i, piles[mid].back())) hi = mid;
            else lo = mid + 1;
        }
        if (lo == static_cast<int>(piles.size())) piles.emplace_back();
        piles[lo].push_back(i);
    }
    for (int k = 0; k < n; ++k) {
        int best = -1;
        for (int p = 0; p < static_cast<int>(piles.size()); ++p) {
            if (piles[p].empty()) continue;
            if (best >= 0) r.compare(piles[p].back(), piles[best].back());
            if (best < 0 || v[piles[p].back()] < v[piles[best].back()]) best = p;
        }
        r.write(k, v[piles[best].back()]);
        piles[best].pop_back();
    }
}
