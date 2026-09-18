#include "recorder.h"

#include <random>

namespace {

constexpr size_t kOpsPerRecord = 60000;

bool bogobogo(Recorder& r, std::mt19937& rng, int n) {
    if (n <= 1) return true;
    for (;;) {
        if (r.ops.size() >= kOpsPerRecord) return false;
        if (!bogobogo(r, rng, n - 1)) return false;
        if (!r.less(n - 1, n - 2)) return true;
        for (int i = n - 1; i > 0; --i) {
            const int j = std::uniform_int_distribution<int>(0, i)(rng);
            if (j != i) r.swap(i, j);
        }
    }
}

}

void bogobogoSort(Recorder& r) {
    static std::mt19937 rng{std::random_device{}()};
    bogobogo(r, rng, r.size());
}
