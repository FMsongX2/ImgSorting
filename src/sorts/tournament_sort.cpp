#include "recorder.h"

#include <vector>

void tournamentSort(Recorder& r) {
    const int n = r.size();
    if (n == 0) return;
    const std::vector<int> v = r.a;
    int size = 1;
    while (size < n) size <<= 1;
    std::vector<int> tree(2 * size, -1);
    for (int i = 0; i < n; ++i) tree[size + i] = i;
    auto winner = [&](int a, int b) {
        if (a < 0) return b;
        if (b < 0) return a;
        r.compare(a, b);
        return v[b] < v[a] ? b : a;
    };
    for (int node = size - 1; node >= 1; --node) tree[node] = winner(tree[2 * node], tree[2 * node + 1]);
    for (int k = 0; k < n; ++k) {
        const int champion = tree[1];
        r.write(k, v[champion]);
        int node = size + champion;
        tree[node] = -1;
        for (node >>= 1; node >= 1; node >>= 1) tree[node] = winner(tree[2 * node], tree[2 * node + 1]);
    }
}
