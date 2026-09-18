#include "recorder.h"

void stalinSort(Recorder& r) {
    int last = -1;
    for (int i = 0; i < r.size(); ++i) {
        if (last >= 0 && r.less(i, last)) r.write(i, kPurged);
        else last = i;
    }
}
