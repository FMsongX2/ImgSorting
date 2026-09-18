#include "sorts.h"

#include "sorts/recorder.h"

#include <utility>

namespace {

constexpr void (*kRunners[])(Recorder&) = {
    bubbleSort,   cocktailShakerSort, oddEvenSort,  gnomeSort,          combSort,    selectionSort, cycleSort,
    insertionSort, pancakeSort,       mergeSort,    blockMergeSort,     inPlaceMergeSort, timSort,  patienceSort,  strandSort,
    quickSort,    pdqSort,            tournamentSort, treeSort,         heapSort,    introSort,     smoothSort,
    shellSort,    countingSort,       pigeonholeSort, bucketSort,       flashSort,   msdRadixSort,  binaryMsdRadixSort,
    radixSort,    gravitySort,        bitonicSort,  stoogeSort,         slowSort,    stalinSort,    bozoSort,
    bogobogoSort, bogoSort,
};
static_assert(std::size(kRunners) == std::size(kSortAlgorithms), "kSortAlgorithms 표와 순서·개수를 맞춤");

}

std::vector<Op> recordSort(int algorithm, std::vector<int> values) {
    Recorder r{std::move(values), {}};
    kRunners[algorithm](r);
    return std::move(r.ops);
}

void applyOp(std::vector<int>& values, const Op& op) {
    if (op.kind == Op::Swap)
        std::swap(values[op.a], values[op.b]);
    else if (op.kind == Op::Write)
        values[op.a] = op.b;
}
