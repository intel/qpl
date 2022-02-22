/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl_api_ref.h"

#include "operation_test.hpp"
#include "ta_ll_common.hpp"
#include "source_provider.hpp"

namespace qpl::test
{
    class CopyTest : public ReferenceFixtureWithTestCases<uint32_t>
    {
    protected:
        void InitializeTestCases() override
        {
            for (uint32_t length = 100; length < 300; length++)
            {
                AddNewTestCase(length);
            }

            for (uint32_t length = 1100; length < 1200; length++)
            {
                AddNewTestCase(length);
            }
        }

        void SetUpBeforeIteration() override
        {
            job_ptr->op           = qpl_op_memcpy;
            reference_job_ptr->op = qpl_op_memcpy;

            source_provider source_generator(GetTestCase(),
                                             8u,
                                             GetSeed());

            ASSERT_NO_THROW(source = source_generator.get_source());
            destination.resize(source.size());
            reference_destination.resize(source.size());

            job_ptr->next_in_ptr   = source.data();
            job_ptr->available_in  = static_cast<uint32_t>(source.size());
            job_ptr->next_out_ptr  = destination.data();
            job_ptr->available_out = static_cast<uint32_t>(destination.size());

            reference_job_ptr->next_in_ptr   = source.data();
            reference_job_ptr->available_in  = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_out_ptr  = reference_destination.data();
            reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(copy, execute_copy, CopyTest)
    {
        auto library_status = run_job_api(job_ptr);

        auto reference_status = ref_copy_8u(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, library_status);
        EXPECT_EQ(library_status, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());

        ASSERT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }
}
