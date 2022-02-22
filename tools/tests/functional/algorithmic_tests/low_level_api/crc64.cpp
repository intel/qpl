/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl_api_ref.h"
#include "../../../common/operation_test.hpp"
#include "ta_ll_common.hpp"
#include "qpl/cpp_api/operations/other/crc_operation.hpp"
#include "../../../common/execution_wrapper.hpp"
#include "format_generator.hpp"
#include "source_provider.hpp"

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
        void SetUpBeforeIteration() override
        {
            CRC64TestCase current_test_case = GetTestCase();

            source_provider source_generator(current_test_case.length,
                                             8u,
                                             GetSeed());

            ASSERT_NO_THROW(source = source_generator.get_source());
            destination.resize(source.size());
            reference_destination.resize(source.size());

            job_ptr->next_in_ptr            = source.data();
            job_ptr->available_in           = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_in_ptr  = source.data();
            reference_job_ptr->available_in = static_cast<uint32_t>(source.size());

            job_ptr->op           = qpl_op_crc64;
            reference_job_ptr->op = qpl_op_crc64;

            if (BE == current_test_case.bit_order)
            {
                job_ptr->flags           = QPL_FLAG_CRC64_BE;
                reference_job_ptr->flags = QPL_FLAG_CRC64_BE;
            }

            if (crc_inverse == current_test_case.crc_direction)
            {
                job_ptr->flags |= QPL_FLAG_CRC64_INV;
                reference_job_ptr->flags |= QPL_FLAG_CRC64_INV;
            }

            poly = format_generator::get_random_crc64_poly(current_test_case.poly_shift,
                                         GetSeed());

            job_ptr->crc64_poly           = poly;
            reference_job_ptr->crc64_poly = poly;
        }


        void InitializeTestCases() override
        {
            for (uint32_t length = 1u; length < 7u; length++)
            {
                for (uint32_t poly_shift : {0u, 32u, 48u})
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

            for (uint32_t length = 8u; length < 63u; length++)
            {
                for (uint32_t poly_shift : {0u, 32u, 48u})
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

            for (uint32_t length = 128u; length < 192u; length++)
            {
                for (uint32_t poly_shift : {0u, 32u, 48u})
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

            for (uint32_t length = 1000u; length < 2000u; length+= 101)
            {
                for (uint32_t poly_shift : {0u, 32u, 48u})
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
}
