#include "recorder.h"

void cocktailShakerSort(Recorder& r) {
    int lo = 0, hi = r.size() - 1;
    while (lo < hi) {
        int lastSwap = lo;
        for (int i = lo; i < hi; ++i)
            if (r.less(i + 1, i)) {
                r.swap(i, i + 1);
                lastSwap = i;
            }
        hi = lastSwap;
        lastSwap = hi;
        for (int i = hi; i > lo; --i)
            if (r.less(i, i - 1)) {
                r.swap(i - 1, i);
                lastSwap = i;
            }
        lo = lastSwap;
    }
}
