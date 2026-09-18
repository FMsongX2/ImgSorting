#include "recorder.h"

#include <random>

namespace {

constexpr int kAttemptsPerRecord = 20000;

}

void bozoSort(Recorder& r) {
    static std::mt19937 rng{std::random_device{}()};
    if (r.size() < 2) return;
    std::uniform_int_distribution<int> pick(0, r.size() - 1);
    for (int attempt = 0; attempt < kAttemptsPerRecord; ++attempt) {
        if (r.sorted()) return;
        const int i = pick(rng);
        int j = pick(rng);
        while (j == i) j = pick(rng);
        r.swap(i, j);
    }
}
