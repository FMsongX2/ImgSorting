#include "recorder.h"

namespace {

void flip(Recorder& r, int k) {
    for (int i = 0, j = k; i < j; ++i, --j) r.swap(i, j);
}

}

void pancakeSort(Recorder& r) {
    for (int size = r.size(); size > 1; --size) {
        int max = 0;
        for (int i = 1; i < size; ++i)
            if (r.less(max, i)) max = i;
        if (max == size - 1) continue;
        if (max > 0) flip(r, max);
        flip(r, size - 1);
    }
}
