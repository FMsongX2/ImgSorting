#include "recorder.h"

#include <vector>

void strandSort(Recorder& r) {
    const int n = r.size();
    for (int done = 0; done < n;) {
        std::vector<int> strand{r.a[done]}, rest;
        int last = done;
        for (int i = done + 1; i < n; ++i) {
            if (r.less(i, last)) {
                rest.push_back(r.a[i]);
            } else {
                strand.push_back(r.a[i]);
                last = i;
            }
        }
        std::vector<int> next;
        next.reserve(n);
        size_t i = 0, j = 0;
        while (i < static_cast<size_t>(done) && j < strand.size()) {
            r.compare(static_cast<int>(i), done);
            next.push_back(strand[j] < r.a[i] ? strand[j++] : r.a[i++]);
        }
        while (i < static_cast<size_t>(done)) next.push_back(r.a[i++]);
        while (j < strand.size()) next.push_back(strand[j++]);
        next.insert(next.end(), rest.begin(), rest.end());
        for (int k = 0; k < n; ++k)
            if (r.a[k] != next[k]) r.write(k, next[k]);
        done += static_cast<int>(strand.size());
    }
}
