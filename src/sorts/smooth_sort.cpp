#include "recorder.h"

#include <bit>
#include <cstdint>

namespace {

constexpr int kLeonardo[] = {1,     1,     3,     5,     9,      15,     25,     41,     67,     109,
                             177,   287,   465,   753,   1219,   1973,   3193,   5167,   8361,   13529,
                             21891, 35421, 57313, 92735, 150049, 242785, 392835, 635621, 1028457};

void sift(Recorder& r, int order, int head) {
    while (order > 1) {
        const int right = head - 1, left = head - 1 - kLeonardo[order - 2];
        if (!r.less(head, left) && !r.less(head, right)) return;
        if (!r.less(left, right)) {
            r.swap(head, left);
            head = left;
            order -= 1;
        } else {
            r.swap(head, right);
            head = right;
            order -= 2;
        }
    }
}

void trinkle(Recorder& r, std::uint64_t trees, int order, int head, bool trusty) {
    while (trees != 1) {
        const int stepson = head - kLeonardo[order];
        if (!r.less(head, stepson)) break;
        if (!trusty && order > 1) {
            const int right = head - 1, left = head - 1 - kLeonardo[order - 2];
            if (!r.less(right, stepson) || !r.less(left, stepson)) break;
        }
        r.swap(head, stepson);
        head = stepson;
        const int trail = std::countr_zero(trees & ~std::uint64_t{1});
        trees >>= trail;
        order += trail;
        trusty = false;
    }
    if (!trusty) sift(r, order, head);
}

}

void smoothSort(Recorder& r) {
    const int n = r.size();
    if (n < 2) return;
    std::uint64_t trees = 1;
    int order = 1, head = 0;
    for (; head < n - 1; ++head) {
        if ((trees & 3) == 3) {
            sift(r, order, head);
            trees >>= 2;
            order += 2;
        } else {
            if (kLeonardo[order - 1] >= n - 1 - head) trinkle(r, trees, order, head, false);
            else sift(r, order, head);
            if (order == 1) {
                trees <<= 1;
                order = 0;
            } else {
                trees <<= order - 1;
                order = 1;
            }
        }
        trees |= 1;
    }
    trinkle(r, trees, order, head, false);
    for (; order != 1 || trees != 1; --head) {
        if (order <= 1) {
            const int trail = std::countr_zero(trees & ~std::uint64_t{1});
            trees >>= trail;
            order += trail;
        } else {
            trees <<= 2;
            trees ^= 7;
            order -= 2;
            trinkle(r, trees >> 1, order + 1, head - kLeonardo[order] - 1, true);
            trinkle(r, trees, order, head - 1, true);
        }
    }
}
