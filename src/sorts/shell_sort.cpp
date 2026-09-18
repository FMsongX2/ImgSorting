#include "recorder.h"

void shellSort(Recorder& r) {
    const int n = r.size();
    int gap = 1;
    while (gap < n / 3) gap = gap * 3 + 1;
    for (; gap > 0; gap /= 3)
        for (int i = gap; i < n; ++i)
            for (int j = i; j >= gap && r.less(j, j - gap); j -= gap) r.swap(j, j - gap);
}
