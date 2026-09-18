#include "recorder.h"

#include <random>

namespace {

constexpr int kAttemptsPerRecord = 600;

}

void bogoSort(Recorder& r) {
    static std::mt19937 rng{std::random_device{}()};
    for (int attempt = 0; attempt < kAttemptsPerRecord; ++attempt) {
        if (r.sorted()) return;
        for (int i = r.size() - 1; i > 0; --i) {
            const int j = std::uniform_int_distribution<int>(0, i)(rng);
            if (j != i) r.swap(i, j);
        }
    }
}
