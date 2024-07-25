/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#if defined(__linux__)

#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>

#ifdef MADV_PAGEOUT

#include <cstdlib>
#include <cstring> // memset, memcmp
#include <gtest/gtest.h>
#include <memory> // unique_ptr

#include "qpl/qpl.h"

#include "ta_ll_common.hpp"

// common
#include "common_defs.hpp"
#include "operation_test.hpp"
#include "system_info.hpp"
#include "util.hpp"

// generators
#include "random_generator.h"

namespace qpl::test {

/**
 * @brief Tests that intentionally swap out page in src or dst,
 * and checks that compression was done w/a errors.
 *
 * @note Tests use madvise that required page aligned memory.
*/
class DeflateWithPageFaults : public JobFixture {
protected:
    testing::AssertionResult RunTestDeflatePageFaults(PageFaultType type) {
        const auto   psize    = getpagesize();
        const size_t src_size = 1U * psize;
        const size_t dst_size = 2U * src_size;

        uint8_t* aligned_src_buffer = static_cast<uint8_t*>(std::aligned_alloc(psize, src_size));
        uint8_t* aligned_dst_buffer = static_cast<uint8_t*>(std::aligned_alloc(psize, dst_size));

        if (aligned_src_buffer == nullptr || aligned_dst_buffer == nullptr) {
            std::free(aligned_src_buffer); //NOLINT(cppcoreguidelines-no-malloc)
            std::free(aligned_dst_buffer); //NOLINT(cppcoreguidelines-no-malloc)

            return testing::AssertionFailure() << "aligned_src_buffer == nullptr || aligned_dst_buffer == nullptr\n";
        }

        qpl::test::random random_element_generator(0, 1, GetSeed());
        for (size_t i = 0U; i < src_size; i++) {
            aligned_src_buffer[i] = (uint8_t)random_element_generator;
        }
        std::memset(aligned_dst_buffer, 0, dst_size);

        const std::unique_ptr<uint8_t, decltype(std::free)*> src {aligned_src_buffer, std::free};
        const std::unique_ptr<uint8_t, decltype(std::free)*> dst {aligned_dst_buffer, std::free};

        job_ptr->op            = qpl_op_compress;
        job_ptr->level         = qpl_default_level;
        job_ptr->next_in_ptr   = src.get();
        job_ptr->next_out_ptr  = dst.get();
        job_ptr->available_in  = src_size;
        job_ptr->available_out = dst_size;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;

        if (is_madv_pageout_available()) {
            int err = 0;

            if (type == READ_SRC_1_PAGE_FAULT) {
                err = madvise(src.get(), psize, MADV_PAGEOUT);
            } else if (type == WRITE_PAGE_FAULT) {
                err = madvise(dst.get(), psize, MADV_PAGEOUT);
            }

            if (err) {
                const int errsv = errno;
                return testing::AssertionFailure() << "madvise failed, error code is " << errsv << "\n";
            }
        } else {
            // if MADV_PAGEOUT is not present, simply return since we have functional testing
            return testing::AssertionSuccess();
        }

        const qpl_status status = run_job_api(job_ptr);
        if (status != QPL_STS_OK) { return testing::AssertionFailure() << "Deflate status is " << status << "\n"; }

        return testing::AssertionSuccess();
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_with_page_fault, read, DeflateWithPageFaults) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    RunTestDeflatePageFaults(READ_SRC_1_PAGE_FAULT);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(deflate_with_page_fault, write, DeflateWithPageFaults) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    RunTestDeflatePageFaults(WRITE_PAGE_FAULT);
}

} // namespace qpl::test

#endif // #ifdef MADV_PAGEOUT
#endif // #if defined(__linux__)
