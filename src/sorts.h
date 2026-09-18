#pragma once

#include <iterator>
#include <string_view>
#include <vector>

struct Op {
    enum Kind : unsigned char { Compare, Swap, Write } kind;
    int a;
    int b;
};

struct SortAlgorithm {
    const char* name;
    const char* complexity;
    bool endless = false;
};

inline constexpr int kPurged = -1;

inline constexpr SortAlgorithm kSortAlgorithms[] = {
    {"Bubble Sort", "O(n^2)"},
    {"Cocktail Shaker Sort", "O(n^2)"},
    {"Odd-Even Sort", "O(n^2)"},
    {"Gnome Sort", "O(n^2)"},
    {"Comb Sort", "O(n^2)"},
    {"Selection Sort", "O(n^2)"},
    {"Cycle Sort", "O(n^2)"},
    {"Insertion Sort", "O(n^2)"},
    {"Pancake Sort", "O(n^2)"},
    {"Merge Sort", "O(n log n)"},
    {"Block Merge Sort", "O(n log n)"},
    {"In-Place Merge Sort", "O(n log^2 n)"},
    {"Tim Sort", "O(n log n)"},
    {"Patience Sort", "O(n log n)"},
    {"Strand Sort", "O(n^2)"},
    {"Quick Sort", "O(n log n)"},
    {"PDQ Sort", "O(n log n)"},
    {"Tournament Sort", "O(n log n)"},
    {"Tree Sort", "O(n log n)"},
    {"Heap Sort", "O(n log n)"},
    {"Intro Sort", "O(n log n)"},
    {"Smooth Sort", "O(n log n)"},
    {"Shell Sort", "O(n^1.5)"},
    {"Counting Sort", "O(n+k)"},
    {"Pigeonhole Sort", "O(n+r)"},
    {"Bucket Sort", "O(n+k)"},
    {"Flash Sort", "O(n+r)"},
    {"Radix Sort (MSD)", "O(n*k/d)"},
    {"Radix Sort (in-place)", "O(n*k)"},
    {"Radix Sort (LSD)", "O(d(n+b))"},
    {"Gravity Sort", "O(S)"},
    {"Bitonic Sort", "O(n log^2 n)"},
    {"Stooge Sort", "O(n^2.71)"},
    {"Slow Sort", "O(n^(log n/2))"},
    {"Stalin Sort", "O(n)"},
    {"Bozo Sort", "O(n!)", true},
    {"Bogobogo Sort", "O(n!^(n-k))", true},
    {"Bogo Sort", "O(n*n!)", true},
};
inline constexpr int kSortCount = static_cast<int>(std::size(kSortAlgorithms));
static_assert(std::string_view(kSortAlgorithms[kSortCount - 1].name) == "Bogo Sort", "보고 정렬은 항상 마지막");
static_assert(
    [] {
        bool endlessSeen = false;
        for (const SortAlgorithm& a : kSortAlgorithms) {
            if (a.endless) endlessSeen = true;
            else if (endlessSeen) return false;
        }
        return true;
    }(),
    "끝나지 않는 정렬은 맨 뒤에 모음");

std::vector<Op> recordSort(int algorithm, std::vector<int> values);

void applyOp(std::vector<int>& values, const Op& op);
