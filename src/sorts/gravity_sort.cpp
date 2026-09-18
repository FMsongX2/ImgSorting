#include "recorder.h"

#include <algorithm>
#include <vector>

void gravitySort(Recorder& r) {
    const int n = r.size();
    if (n == 0) return;
    const std::vector<int> original = r.a;
    const int maxValue = *std::max_element(original.begin(), original.end());
    std::vector<int> fallen(n, 0);
    for (int c = 0; c < maxValue; ++c) {
        int beads = 0;
        for (int v : original) beads += v > c;
        for (int i = n - beads; i < n; ++i) ++fallen[i];
        for (int i = 0; i < n; ++i) {
            const int value = fallen[i] + std::max(0, original[i] - (c + 1));
            if (r.a[i] != value) r.write(i, value);
        }
    }
}
