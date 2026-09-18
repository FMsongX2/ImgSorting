#include "recorder.h"

namespace {

constexpr int kBlockSize = 20;

void swapRange(Recorder& r, int a, int b, int n) {
    for (int i = 0; i < n; ++i) r.swap(a + i, b + i);
}

void rotate(Recorder& r, int a, int m, int b) {
    int i = m - a, j = b - m;
    while (i != j) {
        if (i > j) {
            swapRange(r, m - i, m, j);
            i -= j;
        } else {
            swapRange(r, m - i, m + j - i, i);
            j -= i;
        }
    }
    swapRange(r, m - i, m, i);
}

void symMerge(Recorder& r, int a, int m, int b) {
    if (m - a == 1) {
        int i = m, j = b;
        while (i < j) {
            const int h = (i + j) / 2;
            if (r.less(h, a)) i = h + 1;
            else j = h;
        }
        for (int k = a; k < i - 1; ++k) r.swap(k, k + 1);
        return;
    }
    if (b - m == 1) {
        int i = a, j = m;
        while (i < j) {
            const int h = (i + j) / 2;
            if (!r.less(m, h)) i = h + 1;
            else j = h;
        }
        for (int k = m; k > i; --k) r.swap(k, k - 1);
        return;
    }
    const int mid = (a + b) / 2;
    const int n = mid + m;
    int start = m > mid ? n - b : a;
    int hi = m > mid ? mid : m;
    const int p = n - 1;
    while (start < hi) {
        const int c = (start + hi) / 2;
        if (!r.less(p - c, c)) start = c + 1;
        else hi = c;
    }
    const int end = n - start;
    if (start < m && m < end) rotate(r, start, m, end);
    if (a < start && start < mid) symMerge(r, a, start, mid);
    if (mid < end && end < b) symMerge(r, mid, end, b);
}

}

void inPlaceMergeSort(Recorder& r) {
    const int n = r.size();
    int a = 0, b = kBlockSize;
    for (; b <= n; a = b, b += kBlockSize) insertionSortRange(r, a, b);
    insertionSortRange(r, a, n);
    for (int block = kBlockSize; block < n; block *= 2) {
        a = 0;
        for (b = 2 * block; b <= n; a = b, b += 2 * block) symMerge(r, a, a + block, b);
        if (a + block < n) symMerge(r, a, a + block, n);
    }
}
