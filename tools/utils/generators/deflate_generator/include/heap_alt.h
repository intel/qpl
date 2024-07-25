/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <stdint.h>

// heap array has elements from 1...n
// Make a heap starting with i
// 
static
void
heapify32(uint32_t *heap, uint32_t n, uint32_t i)
{
    uint32_t child, tmp;

    while (true)
    {
        child = 2 * i;
        if (child > n)
            break;
        if ((child < n) && (heap[ child ] > heap[ child + 1 ]))
            child++;
        if (heap[ i ] <= heap[ child ])
            break;
        // swap i and child
        tmp = heap[ i ];
        heap[ i ]     = heap[ child ];
        heap[ child ] = tmp;
        i = child;
    }
}

static void build_heap32(uint32_t *heap, uint32_t n)
{
    uint32_t i;

    for (i = n / 2; i > 0; i--)
    {
        heapify32(heap, n, i);
    }
}

static void replace_min32(uint32_t *heap, uint32_t n, uint32_t new_val)
{
    heap[ 1 ] = new_val;
    heapify32(heap, n, 1);
}

////////////////////////////////////////////////////////////////////////

// heap array has elements from 1...n
// Make a heap starting with i
// 
static
void
heapify64(uint64_t *heap, uint32_t n, uint32_t i)
{
    uint32_t child;
    uint64_t tmp;

    while (true)
    {
        child = 2 * i;
        if (child > n)
            break;
        if (/*(child < n) &&*/ (heap[ child ] > heap[ child + 1 ]))
            child++;
        if (heap[ i ] <= heap[ child ])
            break;
        // swap i and child
        tmp = heap[ i ];
        heap[ i ]     = heap[ child ];
        heap[ child ] = tmp;
        i = child;
    }
}

static void build_heap64(uint64_t *heap, uint32_t n)
{
    uint32_t i;

    heap[ n + 1 ] = 0xFFFFFFFFFFFFFFFF;
    for (i = n / 2; i > 0; i--)
    {
        heapify64(heap, n, i);
    }
}

#define REMOVE_MIN64(heap, n, result) do { \
   (result) = (heap)[1]; \
   (heap)[1] = (heap)[n]; \
   (heap)[(n)--] = 0xFFFFFFFFFFFFFFFF; \
   heapify64(heap, n, 1); \
} while (0)

static void replace_min64(uint64_t *heap, uint32_t n, uint64_t new_val)
{
    heap[ 1 ] = new_val;
    heapify64(heap, n, 1);
}
