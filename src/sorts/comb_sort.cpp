#include "recorder.h"

#include <algorithm>

void combSort(Recorder& r) {
    const int n = r.size();
    int gap = n;
    for (bool swapped = true; gap > 1 || swapped;) {
        gap = std::max(1, static_cast<int>(gap / 1.3));
        swapped = false;
        for (int i = 0; i + gap < n; ++i)
            if (r.less(i + gap, i)) {
                r.swap(i, i + gap);
                swapped = true;
            }
    }
}
