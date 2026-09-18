#include "recorder.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <utility>

namespace {

struct Range {
    int start = 0, end = 0;
    int length() const { return end - start; }
};

int lowerBound(Recorder& r, int first, int last, int value) {
    for (int count = last - first; count > 0;) {
        const int step = count / 2, it = first + step;
        if (r.less(it, value)) {
            first = it + 1;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    return first;
}

int upperBound(Recorder& r, int first, int last, int value) {
    for (int count = last - first; count > 0;) {
        const int step = count / 2, it = first + step;
        if (!r.less(value, it)) {
            first = it + 1;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    return first;
}

int findFirstForward(Recorder& r, int first, int last, int value, int unique) {
    const int size = last - first;
    if (size == 0) return first;
    const int skip = std::max(size / unique, 1);
    int index = first + skip;
    for (; r.less(index - 1, value); index += skip)
        if (index >= last - skip) return lowerBound(r, index, last, value);
    return lowerBound(r, index - skip, index, value);
}

int findLastForward(Recorder& r, int first, int last, int value, int unique) {
    const int size = last - first;
    if (size == 0) return first;
    const int skip = std::max(size / unique, 1);
    int index = first + skip;
    for (; !r.less(value, index - 1); index += skip)
        if (index >= last - skip) return upperBound(r, index, last, value);
    return upperBound(r, index - skip, index, value);
}

int findFirstBackward(Recorder& r, int first, int last, int value, int unique) {
    const int size = last - first;
    if (size == 0) return first;
    const int skip = std::max(size / unique, 1);
    int index = last - skip;
    for (; index > first && !r.less(index - 1, value); index -= skip)
        if (index < first + skip) return lowerBound(r, first, index, value);
    return lowerBound(r, index, index + skip, value);
}

int findLastBackward(Recorder& r, int first, int last, int value, int unique) {
    const int size = last - first;
    if (size == 0) return first;
    const int skip = std::max(size / unique, 1);
    int index = last - skip;
    for (; index > first && r.less(value, index - 1); index -= skip)
        if (index < first + skip) return upperBound(r, first, index, value);
    return upperBound(r, index, index + skip, value);
}

void reverse(Recorder& r, int a, int b) {
    for (--b; a < b; ++a, --b) r.swap(a, b);
}

void rotate(Recorder& r, int a, int m, int b) {
    if (a == m || m == b) return;
    reverse(r, a, m);
    reverse(r, m, b);
    reverse(r, a, b);
}

void swapRanges(Recorder& r, int a, int b, int n) {
    for (int i = 0; i < n; ++i) r.swap(a + i, b + i);
}

void mergeInternal(Recorder& r, int first1, int last1, int first2, int last2, int buffer) {
    int aIndex = buffer, bIndex = first2, insert = first1;
    const int aLast = buffer + (last1 - first1), bLast = last2;
    if (last2 - first2 > 0 && last1 - first1 > 0) {
        for (;;) {
            if (!r.less(bIndex, aIndex)) {
                r.swap(insert++, aIndex++);
                if (aIndex == aLast) break;
            } else {
                r.swap(insert++, bIndex++);
                if (bIndex == bLast) break;
            }
        }
    }
    swapRanges(r, aIndex, insert, aLast - aIndex);
}

void mergeInPlace(Recorder& r, int first1, int last1, int first2, int last2) {
    if (last1 - first1 == 0 || last2 - first2 == 0) return;
    for (;;) {
        const int mid = lowerBound(r, first2, last2, first1);
        const int amount = mid - last1;
        rotate(r, first1, last1, mid);
        if (last2 == mid) break;
        first2 = mid;
        first1 += amount;
        last1 = first2;
        first1 = upperBound(r, first1, last1, first1);
        if (last1 - first1 == 0) break;
    }
}

class WikiIterator {
    std::size_t size, denominator, decimal = 0, numerator = 0, decimalStep, numeratorStep;

public:
    WikiIterator(std::size_t size, std::size_t minLevel)
        : size(size), denominator(std::bit_floor(size) / minLevel), decimalStep(size / denominator),
          numeratorStep(size % denominator) {}
    void begin() { numerator = decimal = 0; }
    Range nextRange() {
        const std::size_t start = decimal;
        decimal += decimalStep;
        numerator += numeratorStep;
        if (numerator >= denominator) {
            numerator -= denominator;
            ++decimal;
        }
        return {static_cast<int>(start), static_cast<int>(decimal)};
    }
    bool finished() const { return decimal >= size; }
    bool nextLevel() {
        decimalStep += decimalStep;
        numeratorStep += numeratorStep;
        if (numeratorStep >= denominator) {
            numeratorStep -= denominator;
            ++decimalStep;
        }
        return decimalStep < size;
    }
    int length() const { return static_cast<int>(decimalStep); }
};

struct Pull {
    int from = 0, to = 0, count = 0;
    Range range;
};

}

void blockMergeSort(Recorder& r) {
    const int size = r.size();
    if (size < 4) {
        if (size == 3) {
            if (r.less(1, 0)) r.swap(0, 1);
            if (r.less(2, 1)) {
                r.swap(1, 2);
                if (r.less(1, 0)) r.swap(0, 1);
            }
        } else if (size == 2) {
            if (r.less(1, 0)) r.swap(0, 1);
        }
        return;
    }

    WikiIterator iterator(size, 4);
    while (!iterator.finished()) {
        int order[] = {0, 1, 2, 3, 4, 5, 6, 7};
        const Range range = iterator.nextRange();
        const int s = range.start;
        auto net = [&](int x, int y) {
            if (r.less(s + y, s + x) || (order[x] > order[y] && !r.less(s + x, s + y))) {
                r.swap(s + x, s + y);
                std::swap(order[x], order[y]);
            }
        };
        switch (range.length()) {
        case 8:
            net(0, 1), net(2, 3), net(4, 5), net(6, 7), net(0, 2), net(1, 3), net(4, 6), net(5, 7), net(1, 2), net(5, 6);
            net(0, 4), net(3, 7), net(1, 5), net(2, 6), net(1, 4), net(3, 6), net(2, 4), net(3, 5), net(3, 4);
            break;
        case 7:
            net(1, 2), net(3, 4), net(5, 6), net(0, 2), net(3, 5), net(4, 6), net(0, 1), net(4, 5), net(2, 6);
            net(0, 4), net(1, 5), net(0, 3), net(2, 5), net(1, 3), net(2, 4), net(2, 3);
            break;
        case 6:
            net(1, 2), net(4, 5), net(0, 2), net(3, 5), net(0, 1), net(3, 4), net(2, 5), net(0, 3), net(1, 4);
            net(2, 4), net(1, 3), net(2, 3);
            break;
        case 5:
            net(0, 1), net(3, 4), net(2, 4), net(2, 3), net(1, 4), net(0, 3), net(0, 2), net(1, 3), net(1, 2);
            break;
        case 4:
            net(0, 1), net(2, 3), net(0, 2), net(1, 3), net(1, 2);
            break;
        }
    }
    if (size < 8) return;

    for (;;) {
        int blockSize = static_cast<int>(std::sqrt(iterator.length()));
        int bufferSize = iterator.length() / blockSize + 1;

        Range buffer1, buffer2;
        int index = 0, last = 0, count = 0, pullIndex = 0;
        Pull pull[2];
        int find = bufferSize + bufferSize;
        bool findSeparately = false;
        if (find > iterator.length()) {
            find = bufferSize;
            findSeparately = true;
        }
        iterator.begin();
        while (!iterator.finished()) {
            const Range A = iterator.nextRange(), B = iterator.nextRange();
            auto recordPull = [&](int to) { pull[pullIndex] = {index, to, count, {A.start, B.end}}; };

            for (last = A.start, count = 1; count < find; last = index, ++count) {
                index = findLastForward(r, last + 1, A.end, last, find - count);
                if (index == A.end) break;
            }
            index = last;
            if (count >= bufferSize) {
                recordPull(A.start);
                pullIndex = 1;
                if (count == bufferSize + bufferSize) {
                    buffer1 = {A.start, A.start + bufferSize};
                    buffer2 = {A.start + bufferSize, A.start + count};
                    break;
                } else if (find == bufferSize + bufferSize) {
                    buffer1 = {A.start, A.start + count};
                    find = bufferSize;
                } else if (findSeparately) {
                    buffer1 = {A.start, A.start + count};
                    findSeparately = false;
                } else {
                    buffer2 = {A.start, A.start + count};
                    break;
                }
            } else if (pullIndex == 0 && count > buffer1.length()) {
                buffer1 = {A.start, A.start + count};
                recordPull(A.start);
            }

            for (last = B.end - 1, count = 1; count < find; last = index - 1, ++count) {
                index = findFirstBackward(r, B.start, last, last, find - count);
                if (index == B.start) break;
            }
            index = last;
            if (count >= bufferSize) {
                recordPull(B.end);
                pullIndex = 1;
                if (count == bufferSize + bufferSize) {
                    buffer1 = {B.end - count, B.end - bufferSize};
                    buffer2 = {B.end - bufferSize, B.end};
                    break;
                } else if (find == bufferSize + bufferSize) {
                    buffer1 = {B.end - count, B.end};
                    find = bufferSize;
                } else if (findSeparately) {
                    buffer1 = {B.end - count, B.end};
                    findSeparately = false;
                } else {
                    if (pull[0].range.start == A.start) pull[0].range.end -= pull[1].count;
                    buffer2 = {B.end - count, B.end};
                    break;
                }
            } else if (pullIndex == 0 && count > buffer1.length()) {
                buffer1 = {B.end - count, B.end};
                recordPull(B.end);
            }
        }

        for (pullIndex = 0; pullIndex < 2; ++pullIndex) {
            Pull& p = pull[pullIndex];
            const int length = p.count;
            if (p.to < p.from) {
                index = p.from;
                for (count = 1; count < length; ++count) {
                    index = findFirstBackward(r, p.to, p.from - (count - 1), index - 1, length - count);
                    rotate(r, index + 1, p.from + 1 - count, p.from + 1);
                    p.from = index + count;
                }
            } else if (p.to > p.from) {
                index = p.from + 1;
                for (count = 1; count < length; ++count) {
                    index = findLastForward(r, index, p.to, index, length - count);
                    rotate(r, p.from, p.from + count, index - 1);
                    p.from = index - count - 1;
                }
            }
        }

        bufferSize = buffer1.length();
        blockSize = iterator.length() / bufferSize + 1;

        iterator.begin();
        while (!iterator.finished()) {
            Range A = iterator.nextRange(), B = iterator.nextRange();

            const int start = A.start;
            bool skip = false;
            for (const Pull& p : pull) {
                if (start != p.range.start) continue;
                if (p.from > p.to) {
                    A.start += p.count;
                    skip |= A.length() == 0;
                } else if (p.from < p.to) {
                    B.end -= p.count;
                    skip |= B.length() == 0;
                }
                if (skip) break;
            }
            if (skip) continue;

            if (r.less(B.end - 1, A.start)) {
                rotate(r, A.start, A.end, B.end);
            } else if (r.less(A.end, A.end - 1)) {
                Range blockA = A;
                const Range firstA{A.start, A.start + blockA.length() % blockSize};

                for (int indexA = buffer1.start, i = firstA.end; i < blockA.end; ++indexA, i += blockSize) r.swap(indexA, i);

                Range lastA = firstA, lastB;
                Range blockB{B.start, B.start + std::min(blockSize, B.length())};
                blockA.start += firstA.length();
                int indexA = buffer1.start;
                if (buffer2.length() > 0) swapRanges(r, lastA.start, buffer2.start, lastA.length());

                if (blockA.length() > 0) {
                    for (;;) {
                        if ((lastB.length() > 0 && !r.less(lastB.end - 1, indexA)) || blockB.length() == 0) {
                            const int bSplit = lowerBound(r, lastB.start, lastB.end, indexA);
                            const int bRemaining = lastB.end - bSplit;
                            int minA = blockA.start;
                            for (int findA = minA + blockSize; findA < blockA.end; findA += blockSize)
                                if (r.less(findA, minA)) minA = findA;
                            swapRanges(r, blockA.start, minA, blockSize);
                            r.swap(blockA.start, indexA++);

                            if (buffer2.length() > 0) mergeInternal(r, lastA.start, lastA.end, lastA.end, bSplit, buffer2.start);
                            else mergeInPlace(r, lastA.start, lastA.end, lastA.end, bSplit);

                            if (buffer2.length() > 0) {
                                swapRanges(r, blockA.start, buffer2.start, blockSize);
                                swapRanges(r, bSplit, blockA.start + blockSize - bRemaining, bRemaining);
                            } else {
                                rotate(r, bSplit, blockA.start, blockA.start + blockSize);
                            }
                            lastA = {blockA.start - bRemaining, blockA.start - bRemaining + blockSize};
                            lastB = {lastA.end, lastA.end + bRemaining};
                            blockA.start += blockSize;
                            if (blockA.length() == 0) break;
                        } else if (blockB.length() < blockSize) {
                            rotate(r, blockA.start, blockB.start, blockB.end);
                            lastB = {blockA.start, blockA.start + blockB.length()};
                            blockA.start += blockB.length();
                            blockA.end += blockB.length();
                            blockB.end = blockB.start;
                        } else {
                            swapRanges(r, blockA.start, blockB.start, blockSize);
                            lastB = {blockA.start, blockA.start + blockSize};
                            blockA.start += blockSize;
                            blockA.end += blockSize;
                            blockB.start += blockSize;
                            if (blockB.end > B.end - blockSize) blockB.end = B.end;
                            else blockB.end += blockSize;
                        }
                    }
                }

                if (buffer2.length() > 0) mergeInternal(r, lastA.start, lastA.end, lastA.end, B.end, buffer2.start);
                else mergeInPlace(r, lastA.start, lastA.end, lastA.end, B.end);
            }
        }

        insertionSortRange(r, buffer2.start, buffer2.end);
        for (Pull& p : pull) {
            int unique = p.count * 2;
            if (p.from > p.to) {
                Range buffer{p.range.start, p.range.start + p.count};
                while (buffer.length() > 0) {
                    index = findFirstForward(r, buffer.end, p.range.end, buffer.start, unique);
                    const int amount = index - buffer.end;
                    rotate(r, buffer.start, buffer.end, index);
                    buffer.start += amount + 1;
                    buffer.end += amount;
                    unique -= 2;
                }
            } else if (p.from < p.to) {
                Range buffer{p.range.end - p.count, p.range.end};
                while (buffer.length() > 0) {
                    index = findLastBackward(r, p.range.start, buffer.start, buffer.end - 1, unique);
                    const int amount = buffer.start - index;
                    rotate(r, index, index + amount, buffer.end);
                    buffer.start -= amount;
                    buffer.end -= amount + 1;
                    unique -= 2;
                }
            }
        }

        if (!iterator.nextLevel()) break;
    }
}
