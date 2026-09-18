#include "recorder.h"

#include <algorithm>
#include <random>

namespace {

constexpr int kKeyDigits = 10;
constexpr long long kKeyLimit = 10'000'000'000;

}

void radixSort(Recorder& r) {
    if (r.a.empty()) return;
    std::vector<long long> key(*std::max_element(r.a.begin(), r.a.end()) + 1);
    std::mt19937_64 rng(key.size());
    std::uniform_int_distribution<long long> pick(0, kKeyLimit - static_cast<long long>(key.size()));
    for (long long& k : key) k = pick(rng);
    std::sort(key.begin(), key.end());
    for (size_t v = 0; v < key.size(); ++v) key[v] += static_cast<long long>(v);
    long long exp = 1;
    for (int digit = 0; digit < kKeyDigits; ++digit, exp *= 10) {
        const std::vector<int> in = r.a;
        int start[11] = {};
        for (int v : in) ++start[key[v] / exp % 10 + 1];
        for (int d = 0; d < 10; ++d) start[d + 1] += start[d];
        for (int v : in) r.write(start[key[v] / exp % 10]++, v);
    }
}
