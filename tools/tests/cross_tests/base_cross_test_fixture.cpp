/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "base_cross_test_fixture.hpp"

void qpl::test::BaseCrossTestFixture::SetUp() {
    m_seed = util::TestEnvironment::GetInstance().GetSeed();
    SetUpHardwareJob();
    SetUpSoftwareJob();
}

void qpl::test::BaseCrossTestFixture::TearDown() {
    qpl_fini_job(sw_job_ptr);
    delete[] m_sw_job_buffer;

    qpl_fini_job(hw_job_ptr);
    delete[] m_hw_job_buffer;
}

uint32_t qpl::test::BaseCrossTestFixture::GetSeed() const {
    return m_seed;
}

void qpl::test::BaseCrossTestFixture::SetUpSoftwareJob() {
    uint32_t job_size = 0;

    auto status = qpl_get_job_size(qpl_path_software, &job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    m_sw_job_buffer = new uint8_t[job_size];
    sw_job_ptr      = reinterpret_cast<qpl_job*>(m_sw_job_buffer);

    status = qpl_init_job(qpl_path_software, sw_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);
}

void qpl::test::BaseCrossTestFixture::SetUpHardwareJob() {
    uint32_t job_size = 0;

    auto status = qpl_get_job_size(qpl_path_hardware, &job_size);
    ASSERT_EQ(QPL_STS_OK, status);

    m_hw_job_buffer = new uint8_t[job_size];
    hw_job_ptr      = reinterpret_cast<qpl_job*>(m_hw_job_buffer);

    status = qpl_init_job(qpl_path_hardware, hw_job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);
}
