#include "recorder.h"

void oddEvenSort(Recorder& r) {
    const int n = r.size();
    for (bool sorted = false; !sorted;) {
        sorted = true;
        for (int start : {1, 0})
            for (int i = start; i + 1 < n; i += 2)
                if (r.less(i + 1, i)) {
                    r.swap(i, i + 1);
                    sorted = false;
                }
    }
}
