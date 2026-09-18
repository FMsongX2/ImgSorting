#pragma once

#include "sorts.h"

#include <utility>
#include <vector>

struct Recorder {
    std::vector<int> a;
    std::vector<Op> ops;

    bool less(int i, int j) {
        ops.push_back({Op::Compare, i, j});
        return a[i] < a[j];
    }
    void compare(int i, int j) { ops.push_back({Op::Compare, i, j}); }
    void swap(int i, int j) {
        ops.push_back({Op::Swap, i, j});
        std::swap(a[i], a[j]);
    }
    void write(int i, int v) {
        ops.push_back({Op::Write, i, v});
        a[i] = v;
    }
    int size() const { return static_cast<int>(a.size()); }
    bool sorted() {
        for (int i = 0; i + 1 < size(); ++i)
            if (less(i + 1, i)) return false;
        return true;
    }
};

void bubbleSort(Recorder& r);
void cocktailShakerSort(Recorder& r);
void oddEvenSort(Recorder& r);
void gnomeSort(Recorder& r);
void combSort(Recorder& r);
void selectionSort(Recorder& r);
void cycleSort(Recorder& r);
void insertionSort(Recorder& r);
void pancakeSort(Recorder& r);
void mergeSort(Recorder& r);
void blockMergeSort(Recorder& r);
void inPlaceMergeSort(Recorder& r);
void timSort(Recorder& r);
void patienceSort(Recorder& r);
void strandSort(Recorder& r);
void quickSort(Recorder& r);
void pdqSort(Recorder& r);
void tournamentSort(Recorder& r);
void treeSort(Recorder& r);
void heapSort(Recorder& r);
void introSort(Recorder& r);
void smoothSort(Recorder& r);
void shellSort(Recorder& r);
void countingSort(Recorder& r);
void pigeonholeSort(Recorder& r);
void bucketSort(Recorder& r);
void flashSort(Recorder& r);
void msdRadixSort(Recorder& r);
void binaryMsdRadixSort(Recorder& r);
void radixSort(Recorder& r);
void gravitySort(Recorder& r);
void bitonicSort(Recorder& r);
void bozoSort(Recorder& r);
void bogobogoSort(Recorder& r);
void stoogeSort(Recorder& r);
void slowSort(Recorder& r);
void stalinSort(Recorder& r);
void bogoSort(Recorder& r);

void mergeAdjacent(Recorder& r, int lo, int mid, int hi);
void insertionSortRange(Recorder& r, int lo, int hi);
void heapSortRange(Recorder& r, int lo, int hi);
