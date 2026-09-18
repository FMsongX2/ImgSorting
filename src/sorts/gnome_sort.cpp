#include "recorder.h"

void gnomeSort(Recorder& r) {
    for (int pos = 0; pos < r.size();) {
        if (pos == 0 || !r.less(pos, pos - 1)) {
            ++pos;
        } else {
            r.swap(pos, pos - 1);
            --pos;
        }
    }
}
