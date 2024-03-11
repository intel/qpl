/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstdlib>
#include <cstdint>
#include <memory>
#include <numeric>
#include <stdbool.h>

#include "qpl/qpl.h"

#include "ta_ll_common.hpp"

// ref
#include "qpl_api_ref.h"

// common
#include "execution_wrapper.hpp"
#include "operation_test.hpp"
#include "system_info.hpp"
#include "util.hpp"
#include "common_defs.hpp"

// utils
#include "source_provider.hpp"

// generator
#include "format_generator.hpp"

#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#endif

namespace qpl::test
{
    enum BitOrder
    {
        LE,
        BE
    };

    enum CRCDirection
    {
        crc_forward,
        crc_inverse
    };

    struct CRC64TestCase
    {
        uint32_t     length;
        uint32_t     poly_shift;
        BitOrder     bit_order;
        CRCDirection crc_direction;
        size_t       src_alignment = 0U;
    };

    std::ostream &operator<<(std::ostream &os, const CRC64TestCase &test_case)
    {
        std::string crc_direction = (test_case.crc_direction == crc_forward) ? "forward" : "inverse";
        std::string bit_order     = (test_case.bit_order == LE) ? "LE" : "BE";
        os << "Source length : " << test_case.length << std::endl;
        os << "Polynomial shift : " << test_case.poly_shift << std::endl;
        os << "CRC direction : " << crc_direction << std::endl;
        os << "Bit order : " << bit_order << std::endl;
        return os;
    }

    class CRC64Test : public ReferenceFixtureWithTestCases<CRC64TestCase>
    {
    protected:
        void SetUpJobs(BitOrder     bit_order,
                       CRCDirection crc_direction,
                       uint32_t     poly_shift,
                       uint8_t*     source,
                       uint32_t     source_size)
        {
            job_ptr->next_in_ptr            = source;
            job_ptr->available_in           = source_size;
            reference_job_ptr->next_in_ptr  = source;
            reference_job_ptr->available_in = source_size;

            job_ptr->op           = qpl_op_crc64;
            reference_job_ptr->op = qpl_op_crc64;

            if (BE == bit_order)
            {
                job_ptr->flags           = QPL_FLAG_CRC64_BE;
                reference_job_ptr->flags = QPL_FLAG_CRC64_BE;
            }

            if (crc_inverse == crc_direction)
            {
                job_ptr->flags |= QPL_FLAG_CRC64_INV;
                reference_job_ptr->flags |= QPL_FLAG_CRC64_INV;
            }

            poly = format_generator::get_random_crc64_poly(poly_shift,
                                                           GetSeed());

            job_ptr->crc64_poly           = poly;
            reference_job_ptr->crc64_poly = poly;
        }

        void SetUpBeforeIteration() override
        {
            CRC64TestCase current_test_case = GetTestCase();

            source_provider source_generator(current_test_case.length,
                                             8U,
                                             GetSeed());

            ASSERT_NO_THROW(source = source_generator.get_source());
            destination.resize(source.size());
            reference_destination.resize(source.size());

            SetUpJobs(current_test_case.bit_order,
                      current_test_case.crc_direction,
                      current_test_case.poly_shift,
                      source.data(),
                      static_cast<uint32_t>(source.size()));
        }

        void InitializeTestCases() override
        {
            for (uint32_t length = 1U; length < 7U; length++)
            {
                for (uint32_t poly_shift : {0U, 32U, 48U})
                {
                    for (auto bit_order : {BitOrder::LE, BitOrder::BE})
                    {
                        for (auto crc_direction : {CRCDirection::crc_forward, CRCDirection::crc_inverse})
                        {
                            CRC64TestCase test_case;
                            test_case.length        = length;
                            test_case.poly_shift    = poly_shift;
                            test_case.bit_order     = bit_order;
                            test_case.crc_direction = crc_direction;

                            AddNewTestCase(test_case);
                        }
                    }
                }
            }

            for (uint32_t length = 8U; length < 63U; length++)
            {
                for (uint32_t poly_shift : {0U, 32U, 48U})
                {
                    for (auto bit_order : {BitOrder::LE, BitOrder::BE})
                    {
                        for (auto crc_direction : {CRCDirection::crc_forward, CRCDirection::crc_inverse})
                        {
                            CRC64TestCase test_case;
                            test_case.length = length;
                            test_case.poly_shift = poly_shift;
                            test_case.bit_order = bit_order;
                            test_case.crc_direction = crc_direction;

                            AddNewTestCase(test_case);
                        }
                    }
                }
            }

            for (uint32_t length = 128U; length < 192U; length++)
            {
                for (uint32_t poly_shift : {0U, 32U, 48U})
                {
                    for (auto bit_order : {BitOrder::LE, BitOrder::BE})
                    {
                        for (auto crc_direction : {CRCDirection::crc_forward, CRCDirection::crc_inverse})
                        {
                            CRC64TestCase test_case;
                            test_case.length = length;
                            test_case.poly_shift = poly_shift;
                            test_case.bit_order = bit_order;
                            test_case.crc_direction = crc_direction;

                            AddNewTestCase(test_case);
                        }
                    }
                }
            }

            for (uint32_t length = 1000U; length < 2000U; length+= 101)
            {
                for (uint32_t poly_shift : {0U, 32U, 48U})
                {
                    for (auto bit_order : {BitOrder::LE, BitOrder::BE})
                    {
                        for (auto crc_direction : {CRCDirection::crc_forward, CRCDirection::crc_inverse})
                        {
                            CRC64TestCase test_case;
                            test_case.length = length;
                            test_case.poly_shift = poly_shift;
                            test_case.bit_order = bit_order;
                            test_case.crc_direction = crc_direction;

                            AddNewTestCase(test_case);
                        }
                    }
                }
            }
        }

        uint64_t poly = 0;
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(crc64, execute_crc64, CRC64Test)
    {
        auto library_status   = run_job_api(job_ptr);
        auto reference_status = ref_crc64(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, library_status);
        EXPECT_EQ(reference_status, library_status);
        ASSERT_EQ(reference_job_ptr->crc64, job_ptr->crc64);
    }

#if defined(__linux__)
#ifdef MADV_PAGEOUT

    class CRC64TestPageFault : public CRC64Test
    {
        protected:
        void SetUpBeforeIteration() override
        {
            CRC64TestCase current_test_case = GetTestCase();

            // allocate extra space in source buffer to make enough room for the aligned address
            source.resize(current_test_case.length + current_test_case.src_alignment - 1U);

            // not in use
            destination.resize(1U);
            reference_destination.resize(1U);

            // align address to allow for madvise usage
            uint8_t* aligned_src = nullptr;
            align_ptr(current_test_case.src_alignment, source.data(), (void **)&aligned_src);

            SetUpJobs(current_test_case.bit_order,
                      current_test_case.crc_direction,
                      current_test_case.poly_shift,
                      aligned_src,
                      current_test_case.length);
        }

        void InitializeTestCases() override
        {
            // not required to test various lengths for page fault testing
            // might be extended in the future
            uint32_t length = 128U;

            for (uint32_t poly_shift : {0U, 32U, 48U})
            {
                for (auto bit_order : {BitOrder::LE, BitOrder::BE})
                {
                    for (auto crc_direction : {CRCDirection::crc_forward, CRCDirection::crc_inverse})
                    {
                        CRC64TestCase test_case;
                        test_case.length = length;
                        test_case.poly_shift = poly_shift;
                        test_case.bit_order = bit_order;
                        test_case.crc_direction = crc_direction;
                        test_case.src_alignment = getpagesize();

                        AddNewTestCase(test_case);
                    }
                }
            }
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(crc64_with_page_fault, read, CRC64TestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);

        if (is_madv_pageout_available()) {
            // swap out a single page of source buffer
            const auto psize = getpagesize();
            int err = madvise(job_ptr->next_in_ptr, psize, MADV_PAGEOUT);
            if (err) {
                int errsv = errno;
                ASSERT_EQ(err, 0) << "madvise failed, error code is " << errsv << "\n";
            }
        }

        auto library_status   = run_job_api(job_ptr);
        auto reference_status = ref_crc64(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, library_status);
        EXPECT_EQ(reference_status, library_status);
        ASSERT_EQ(reference_job_ptr->crc64, job_ptr->crc64);
    }
#endif // #ifdef MADV_PAGEOUT
#endif // #if defined(__linux__)

} // namespace qpl::test
