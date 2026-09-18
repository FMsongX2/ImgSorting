#include "recorder.h"

#include <algorithm>

void countingSort(Recorder& r) {
    if (r.a.empty()) return;
    std::vector<int> count(*std::max_element(r.a.begin(), r.a.end()) + 1);
    for (int v : r.a) ++count[v];
    int k = 0;
    for (int v = 0; v < static_cast<int>(count.size()); ++v)
        for (int c = 0; c < count[v]; ++c) r.write(k++, v);
}
