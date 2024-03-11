/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <array>

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "t_common.hpp"

#include "qplc_deflate_utils.h"
#include "own_qplc_defs.h"

#include "own_defs.h"
#include "deflate_hash_table.h"
#include "deflate_histogram.h"
#include "qpl/c_api/defs.h"
#include "qplc_checksum.h"

#include "deflate_slow_utils.h"
#include "deflate_slow_matcher.h"
#include "deflate_defs.h"
#include "dispatcher.hpp"

constexpr uint32_t D_SIZE_HASH_TABLE  = 4096;
constexpr uint32_t D_SIZE_HASH_STORE  = 4096;
constexpr uint32_t D_SIZE_DEFLATE_ICF = 4096;

static uint32_t     hash_table[D_SIZE_HASH_TABLE];
static uint32_t     hash_story[D_SIZE_HASH_TABLE];

static deflate_icf  p_deflate_icf[D_SIZE_DEFLATE_ICF];

static const uint32_t table_offset[30] = {
    1, 2, 3, 4, 5, 7, 9, 13,
    17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073,
    4097, 6145, 8193, 12289, 16385, 24577
};

static inline qplc_slow_deflate_icf_body_t_ptr qplc_slow_deflate_icf_body() {
    static const auto &table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_deflate_table();

    return (qplc_slow_deflate_icf_body_t_ptr)table[0];
}

static void init_hash_table(void)
{
    for (uint32_t indx = 0; indx < D_SIZE_HASH_TABLE; indx++) {
        hash_table[indx] = 0x80000000;
    }
    for (uint32_t indx = 0; indx < D_SIZE_HASH_STORE; indx++) {
        hash_story[indx] = 0x00000000;
    }
}

static void init_histogram(isal_mod_hist *str_histogram_ptr)
{
    uint32_t* d_hist = str_histogram_ptr->d_hist;
    uint32_t* ll_hist = str_histogram_ptr->ll_hist;
    for (uint32_t indx = 0; indx < 0x1e; indx++) {
        d_hist[indx] = 0;
    }
    for (uint32_t indx = 0; indx < 0x201; indx++) {
        ll_hist[indx] = 0;
    }
}

static void init_deflate_icf(void)
{
    for (uint32_t indx = 0; indx < D_SIZE_DEFLATE_ICF; indx++) {
        p_deflate_icf[indx].lit_len = 0;
        p_deflate_icf[indx].lit_dist = 0;
        p_deflate_icf[indx].dist_extra = 0;
    }
}

static void init_icf_stream(deflate_icf_stream* icf_stream_ptr)
{
    icf_stream_ptr->begin_ptr = p_deflate_icf;
    icf_stream_ptr->next_ptr = p_deflate_icf;
    icf_stream_ptr->end_ptr = p_deflate_icf + D_SIZE_DEFLATE_ICF;
}


static void init_all(isal_mod_hist* str_histogram_ptr, isal_mod_hist* str_histogram_ref_ptr, deflate_icf_stream*  icf_stream_ptr)
{
    init_hash_table();
    init_histogram(str_histogram_ptr);
    init_histogram(str_histogram_ref_ptr);
    init_deflate_icf();
    init_icf_stream(icf_stream_ptr);
}

static uint32_t test_dedeflate_icf_body(deflate_icf_stream *icf_stream_ptr,
    isal_mod_hist *str_histogram_ref, uint8_t *buffer_ptr, uint32_t length)
{
    uint32_t*   icf_ptr = (uint32_t*)icf_stream_ptr->begin_ptr;
    uint32_t    indx;
    uint32_t    icf;
    uint32_t    result = 0;
    for (indx = 0; indx < length;) {
        icf = *icf_ptr++;
        if ((icf & 0x3ff) < 0x100) {
            if ((icf >> 10) != 0x1e) {
                return 1;
            }
            icf &= 0xff;
            buffer_ptr[indx++] = icf;
            str_histogram_ref->ll_hist[icf]++;
            continue;
        }
        if (0x100 == (icf & 0x3ff)) {
            str_histogram_ref->ll_hist[0x100]++;
            break;
        }
        {
            uint32_t    len_match = (icf & 0x3ff) - 0xfe;
            uint32_t    distance = (icf >> 10) & 0x1ff;
            uint32_t    extend = icf >> 19;
            str_histogram_ref->ll_hist[icf & 0x3ff]++;
            str_histogram_ref->d_hist[distance]++;
            distance = table_offset[distance];
            distance += extend;
            for (uint32_t count_byte = 0; count_byte < len_match; indx++, count_byte++) {
                buffer_ptr[indx] = buffer_ptr[indx - distance];
            }
        }
    }
    return result;
}

static uint32_t test_histogram(isal_mod_hist* str_histogram, isal_mod_hist* str_histogram_ref)
{
    uint32_t *d_hist  = str_histogram->d_hist;
    uint32_t* ll_hist = str_histogram->ll_hist;
    uint32_t* d_hist_ref = str_histogram_ref->d_hist;
    uint32_t* ll_hist_ref = str_histogram_ref->ll_hist;

    for (uint32_t indx = 0; indx < 0x1e; indx++) {
        if (d_hist[indx] != d_hist_ref[indx]) {
            return 1;
        }
    }
    for (uint32_t indx = 0; indx < 0x201; indx++) {
        if (ll_hist[indx] != ll_hist_ref[indx]) {
            return 1;
        }
    }
    return 0;
}

static uint32_t test_source(const uint8_t* source, const uint8_t* destination, uint32_t length)
{
    for (uint32_t indx = 0; indx < length; indx++) {
        if (source[indx] != destination[indx]) {
            return 1;
        }
    }
    return 0;

}

constexpr uint32_t TEST_BUFFER_SIZE = 4096U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_deflate_slow_icf, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE> source{};
    std::array<uint8_t, TEST_BUFFER_SIZE> destination{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0U, static_cast<double>(UINT8_MAX), seed);

    uint8_t                 *current_ptr = (uint8_t*)source.data();
    uint8_t                 *lower_bound_ptr = (uint8_t*)source.data();
    uint8_t              *upper_bound_ptr = lower_bound_ptr + source.size();
    deflate_hash_table_t str_hash_table;
    isal_mod_hist        str_histogram;
    isal_mod_hist           str_histogram_ref;
    deflate_icf_stream      icf_stream;
    uint32_t                compressed_bytes_0;
    uint32_t                compressed_bytes_1;
    uint32_t                result;

    str_hash_table.hash_table_ptr = hash_table;
    str_hash_table.hash_story_ptr = hash_story;
    str_hash_table.hash_mask      = 0x0fff;
    str_hash_table.attempts       = 0x1000;
    str_hash_table.good_match     = 0x0020;
    str_hash_table.nice_match     = 0x0102;
    str_hash_table.lazy_match     = 0x0102;

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)indx;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x100);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_TRUE(0 == result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x101);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

    /****************************************************************/

    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x10);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x11);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

    /****************************************************************/

    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        p_source_buf[0xd28] = (uint8_t)0xa5;
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x10);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x11);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

    /****************************************************************/

    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)indx;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 0; indx < 254; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 256; indx < 256 + 255; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 256 + 255 + 2; indx < 256 + 255 + 32; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x300);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x301);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

    /****************************************************************/

    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)indx;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 0; indx < 258; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 258 + 1; indx < 258 + 1 + 256; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 600; indx < 600 + 257; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 1024; indx < 1024 + 258; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 2048);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 2049);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

/******************************************************************************/
/******************************************************************************/

    str_hash_table.good_match = 0x0010;
    str_hash_table.nice_match = 0x020;
    str_hash_table.lazy_match = 0x020;


    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)indx;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x100);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result  = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x101);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

/****************************************************************/

    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x10);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x11);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

    /****************************************************************/

    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)indx;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 0; indx < 254; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 256; indx < 256+255; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 256 + 255 + 2; indx < 256 + 255 + 32; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x300);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x301);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

    /****************************************************************/

    icf_stream.begin_ptr = &p_deflate_icf[0];
    icf_stream.next_ptr = &p_deflate_icf[0];
    icf_stream.end_ptr = &p_deflate_icf[D_SIZE_DEFLATE_ICF];

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)indx;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 0; indx < 258; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 258+1; indx < 258 + 1 + 256; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 600;  indx < 600 + 257; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
        for (uint32_t indx = 1024; indx < 1024 + 258; indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 2048);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 2049);
    compressed_bytes_1 = qplc_slow_deflate_icf_body()(current_ptr + compressed_bytes_0,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    init_histogram(&str_histogram_ref);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0 + compressed_bytes_1);
    ASSERT_EQ(0, result);

    /****************************************************************/

    str_hash_table.good_match = 0x0020;
    str_hash_table.nice_match = 0x0102;
    str_hash_table.lazy_match = 0x0102;

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)indx;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }
    upper_bound_ptr = lower_bound_ptr + 0x100;

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x400);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);


    /****************************************************************/

    str_hash_table.good_match = 0x0020;
    str_hash_table.nice_match = 0x0102;
    str_hash_table.lazy_match = 0x0102;

    init_all(&str_histogram, &str_histogram_ref, &icf_stream);

    {
        uint8_t* p_source_buf = (uint8_t*)source.data();
        uint8_t* p_destination_buf = (uint8_t*)destination.data();
        for (uint32_t indx = 0; indx < source.size(); indx++) {
            p_source_buf[indx] = (uint8_t)0x5a;
            p_destination_buf[indx] = (uint8_t)0x00;
        }
    }
    upper_bound_ptr = lower_bound_ptr + 0x100;

    icf_stream.end_ptr = (deflate_icf*)((uint32_t*)p_deflate_icf + 0x400);
    compressed_bytes_0 = qplc_slow_deflate_icf_body()(current_ptr,
        lower_bound_ptr, upper_bound_ptr, &str_hash_table, &str_histogram,
        &icf_stream);
    test_dedeflate_icf_body(&icf_stream, &str_histogram_ref, (uint8_t*)destination.data(), compressed_bytes_0);
    result = test_histogram(&str_histogram, &str_histogram_ref);
    result |= test_source((uint8_t*)source.data(), (uint8_t*)destination.data(), compressed_bytes_0);
    ASSERT_EQ(0, result);
}
}
