/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <stdint.h>

// heap array has elements from 1...n
// Make a heap starting with i
//
static void heapify32(uint32_t* heap, uint32_t n, uint32_t i) {
    uint32_t child = 0U, tmp = 0U;

    while (true) {
        child = 2U * i;
        if (child > n) break;
        if ((child < n) && (heap[child] > heap[child + 1U])) child++;
        if (heap[i] <= heap[child]) break;
        // swap i and child
        tmp         = heap[i];
        heap[i]     = heap[child];
        heap[child] = tmp;
        i           = child;
    }
}

static void build_heap32(uint32_t* heap, uint32_t n) {
    for (uint32_t i = n / 2; i > 0U; i--) {
        heapify32(heap, n, i);
    }
}

static void replace_min32(uint32_t* heap, uint32_t n, uint32_t new_val) {
    heap[1] = new_val;
    heapify32(heap, n, 1);
}

////////////////////////////////////////////////////////////////////////

// heap array has elements from 1...n
// Make a heap starting with i
//
static void heapify64(uint64_t* heap, uint32_t n, uint32_t i) {
    uint32_t child = 0;
    uint64_t tmp   = 0;

    while (true) {
        child = 2U * i;
        if (child > n) break;
        if (/*(child < n) &&*/ (heap[child] > heap[child + 1U])) child++;
        if (heap[i] <= heap[child]) break;
        // swap i and child
        tmp         = heap[i];
        heap[i]     = heap[child];
        heap[child] = tmp;
        i           = child;
    }
}

static void build_heap64(uint64_t* heap, uint32_t n) {
    heap[n + 1U] = 0xFFFFFFFFFFFFFFFF;
    for (uint32_t i = n / 2; i > 0U; i--) {
        heapify64(heap, n, i);
    }
}

#define REMOVE_MIN64(heap, n, result)                 \
    do { /*NOLINT(cppcoreguidelines-avoid-do-while)*/ \
        (result)      = (heap)[1];                    \
        (heap)[1]     = (heap)[n];                    \
        (heap)[(n)--] = 0xFFFFFFFFFFFFFFFF;           \
        heapify64(heap, n, 1);                        \
    } while (0)

static void replace_min64(uint64_t* heap, uint32_t n, uint64_t new_val) {
    heap[1] = new_val;
    heapify64(heap, n, 1);
}
