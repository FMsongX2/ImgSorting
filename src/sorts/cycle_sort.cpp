#include "recorder.h"

namespace {

int rankOf(Recorder& r, int start, int item) {
    int pos = start;
    for (int i = start + 1; i < r.size(); ++i) {
        r.compare(i, start);
        if (r.a[i] < item) ++pos;
    }
    return pos;
}

}

void cycleSort(Recorder& r) {
    for (int start = 0; start + 1 < r.size(); ++start) {
        int item = r.a[start];
        int pos = rankOf(r, start, item);
        if (pos == start) continue;
        for (;;) {
            const int displaced = r.a[pos];
            r.write(pos, item);
            if (pos == start) break;
            item = displaced;
            pos = rankOf(r, start, item);
        }
    }
}
