#include "recorder.h"

void bubbleSort(Recorder& r) {
    for (int end = r.size() - 1; end > 0; --end) {
        bool swapped = false;
        for (int j = 0; j < end; ++j)
            if (r.less(j + 1, j)) {
                r.swap(j, j + 1);
                swapped = true;
            }
        if (!swapped) return;
    }
}
