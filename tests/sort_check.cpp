#include "sorts.h"

#include <algorithm>
#include <cstdio>
#include <numeric>
#include <random>
#include <string_view>

static std::vector<int> stalinExpected(const std::vector<int>& before) {
    std::vector<int> result(before.size(), kPurged);
    int last = -1;
    for (size_t i = 0; i < before.size(); ++i)
        if (last < 0 || before[i] >= last) result[i] = last = before[i];
    return result;
}

int main() {
    std::mt19937 rng(42);
    int failures = 0;
    for (int n : {0, 1, 2, 5, 7, 100, 257}) {
        std::vector<int> sorted(n);
        std::iota(sorted.begin(), sorted.end(), 0);
        std::vector<int> organPipe;
        for (int v = 0; v < n; v += 2) organPipe.push_back(v);
        for (int v = n - 1 - (n % 2 == 0 ? 0 : 1); v > 0; v -= 2) organPipe.push_back(v);
        std::vector<int> shuffled = sorted;
        std::shuffle(shuffled.begin(), shuffled.end(), rng);
        const std::vector<int> inputs[] = {shuffled, sorted, {sorted.rbegin(), sorted.rend()}, organPipe};
        for (int algo = 0; algo < kSortCount; ++algo)
        for (const std::vector<int>& input : inputs) {
            const SortAlgorithm& info = kSortAlgorithms[algo];
            if (info.endless && n > 5) continue;
            if (std::string_view(info.name) == "Slow Sort" && n > 100) continue;
            std::vector<int> values = input;
            const std::vector<int> before = values;
            bool opsValid = true;
            for (int round = 0; round < (info.endless ? 1000 : 1); ++round) {
                if (round > 0 && values == sorted) break;
                for (const Op& op : recordSort(algo, values)) {
                    const bool aOk = op.a >= 0 && op.a < n;
                    const bool bOk = op.kind == Op::Write ? (op.b == kPurged || (op.b >= 0 && op.b < n)) : (op.b >= 0 && op.b < n);
                    if (!aOk || !bOk) {
                        opsValid = false;
                        break;
                    }
                    applyOp(values, op);
                }
                if (!opsValid) break;
            }
            const bool ok = opsValid && (std::string_view(info.name) == "Stalin Sort" ? values == stalinExpected(before) : values == sorted);
            if (!ok) {
                std::printf("FAIL %s n=%d input#%d\n", info.name, n, static_cast<int>(&input - inputs));
                ++failures;
            }
        }
    }
    std::puts(failures ? "sort_check failed" : "sort_check ok");
    return failures ? 1 : 0;
}
