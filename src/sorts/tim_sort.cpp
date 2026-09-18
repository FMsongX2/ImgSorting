#include "recorder.h"

#include <algorithm>

namespace {

int minRun(int n) {
    int bit = 0;
    while (n >= 64) {
        bit |= n & 1;
        n >>= 1;
    }
    return n + bit;
}

void reverse(Recorder& r, int lo, int hi) {
    for (int i = lo, j = hi - 1; i < j; ++i, --j) r.swap(i, j);
}

int naturalRunEnd(Recorder& r, int lo, int n) {
    int end = lo + 1;
    if (end == n) return end;
    if (r.less(end, lo)) {
        while (end + 1 < n && r.less(end + 1, end)) ++end;
        reverse(r, lo, end + 1);
    } else {
        while (end + 1 < n && !r.less(end + 1, end)) ++end;
    }
    return end + 1;
}

void binaryInsertion(Recorder& r, int lo, int sorted, int hi) {
    for (int i = sorted; i < hi; ++i) {
        int left = lo, right = i;
        while (left < right) {
            const int mid = (left + right) / 2;
            if (r.less(i, mid)) right = mid;
            else left = mid + 1;
        }
        for (int j = i; j > left; --j) r.swap(j, j - 1);
    }
}

}

void timSort(Recorder& r) {
    const int n = r.size();
    if (n < 2) return;
    const int minrun = minRun(n);
    std::vector<int> bounds{0};
    for (int lo = 0; lo < n;) {
        const int natural = naturalRunEnd(r, lo, n);
        const int end = std::min(n, std::max(natural, lo + minrun));
        binaryInsertion(r, lo, natural, end);
        bounds.push_back(lo = end);
    }
    while (bounds.size() > 2) {
        std::vector<int> merged{0};
        for (size_t i = 0; i + 1 < bounds.size(); i += 2) {
            if (i + 2 < bounds.size()) mergeAdjacent(r, bounds[i], bounds[i + 1], bounds[i + 2]);
            merged.push_back(bounds[std::min(i + 2, bounds.size() - 1)]);
        }
        bounds = std::move(merged);
    }
}
