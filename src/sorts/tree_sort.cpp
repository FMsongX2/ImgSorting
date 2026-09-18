#include "recorder.h"

#include <vector>

void treeSort(Recorder& r) {
    const int n = r.size();
    if (n == 0) return;
    struct Node {
        int index;
        int left = -1, right = -1;
    };
    std::vector<Node> nodes{{0}};
    nodes.reserve(n);
    for (int i = 1; i < n; ++i) {
        int cur = 0;
        for (;;) {
            const bool goLeft = r.less(i, nodes[cur].index);
            const int next = goLeft ? nodes[cur].left : nodes[cur].right;
            if (next >= 0) {
                cur = next;
                continue;
            }
            (goLeft ? nodes[cur].left : nodes[cur].right) = static_cast<int>(nodes.size());
            nodes.push_back({i});
            break;
        }
    }
    std::vector<int> sorted;
    sorted.reserve(n);
    std::vector<int> stack;
    for (int cur = 0; cur >= 0 || !stack.empty();) {
        if (cur >= 0) {
            stack.push_back(cur);
            cur = nodes[cur].left;
            continue;
        }
        cur = stack.back();
        stack.pop_back();
        sorted.push_back(r.a[nodes[cur].index]);
        cur = nodes[cur].right;
    }
    for (int k = 0; k < n; ++k) r.write(k, sorted[k]);
}
