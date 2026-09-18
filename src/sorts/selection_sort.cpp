#include "recorder.h"

void selectionSort(Recorder& r) {
    for (int i = 0; i + 1 < r.size(); ++i) {
        int min = i;
        for (int j = i + 1; j < r.size(); ++j)
            if (r.less(j, min)) min = j;
        if (min != i) r.swap(i, min);
    }
}
