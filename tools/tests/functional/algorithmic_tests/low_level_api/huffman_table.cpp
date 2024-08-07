/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include <algorithm>
#include <array>
#include <string>

#include "ta_ll_common.hpp"

// c_api
#include "qpl/c_api/huffman_table.h"
#include "qpl/c_api/serialization.h"

// tests_common
#include "operation_test.hpp"

// tool_common
#include "compare_huffman_table.hpp"
#include "util.hpp"

// tool_generator
#include "random_generator.h"

namespace qpl::test {

enum compression_algorithm_e {
    compression_algorithm_deflate      = 0,
    compression_algorithm_huffman_only = 1,
    compression_algorithm_canned       = 2,
};

struct huffman_table_test_case_t {
    qpl_huffman_table_type_e c_type;
    qpl_huffman_table_type_e d_type;
    compression_algorithm_e  algorithm;
    serialization_options_t  options;
};

std::ostream& operator<<(std::ostream& os, const huffman_table_test_case_t& test_case) {
    std::array<std::string, 3> type               = {"combined_table", "compression_table", "decompression_table"};
    std::array<std::string, 3> algorithm          = {"deflate", "huffman_only", "canned"};
    std::array<std::string, 2> serialization_type = {"compact", "raw"};

    os << '\n';

    os << "\t-- Compression table type: " << type[test_case.c_type] << '\n'
       << "\t-- Decompression table type: " << type[test_case.d_type] << '\n'
       << "\t-- Algorithm: " << algorithm[test_case.algorithm] << '\n'
       << "\t-- Serialization Format: " << serialization_type[test_case.options.format] << '\n'
       << "\t-- Serialization Flag: " << test_case.options.flags << '\n';

    return os;
}

class HuffmanTableAlgorithmicTest : public JobFixtureWithTestCases<huffman_table_test_case_t> {
public:
    void SetUpBeforeIteration() override {
        qpl::test::random value(0, 255, GetSeed());
        qpl::test::random length(4096, 4096 * 8, GetSeed());

        source.resize(static_cast<size_t>(length));

        std::generate(source.begin(), source.end(), [&value]() { return static_cast<uint8_t>(value); });

        destination.resize(source.size());
        m_compressed_data.resize(source.size() * 2U);
    }

    void InitializeTestCases() override {
        constexpr std::array<qpl_huffman_table_type_e, 2> c_types    = {combined_table_type, compression_table_type};
        constexpr std::array<qpl_huffman_table_type_e, 2> d_types    = {combined_table_type, decompression_table_type};
        constexpr std::array<compression_algorithm_e, 3>  algorithms = {
                compression_algorithm_deflate, compression_algorithm_canned, compression_algorithm_huffman_only};
        constexpr std::array<qpl_serialization_format_e, 2> s_types = {serialization_compact, serialization_raw};

        for (auto& algorithm : algorithms) {
            for (auto& type_c : c_types) {
                for (auto& type_d : d_types) {
                    for (auto& type_s : s_types) {
                        huffman_table_test_case_t test_case {};

                        test_case.algorithm      = algorithm;
                        test_case.c_type         = type_c;
                        test_case.d_type         = type_d;
                        test_case.options.format = type_s; // no serialization flags are currently supported

                        AddNewTestCase(test_case);
                    }
                }
            }
        }
    }

    HuffmanTableAlgorithmicTest() {};
    HuffmanTableAlgorithmicTest(const HuffmanTableAlgorithmicTest&)             = delete;
    HuffmanTableAlgorithmicTest(const HuffmanTableAlgorithmicTest&&)            = delete;
    HuffmanTableAlgorithmicTest& operator=(const HuffmanTableAlgorithmicTest&)  = delete;
    HuffmanTableAlgorithmicTest& operator=(const HuffmanTableAlgorithmicTest&&) = delete;
    ~HuffmanTableAlgorithmicTest() override {
        if (m_c_huffman_table) {
            qpl_huffman_table_destroy(m_c_huffman_table);
            m_c_huffman_table = nullptr;
        }

        if (m_d_huffman_table) {
            qpl_huffman_table_destroy(m_d_huffman_table);
            m_d_huffman_table = nullptr;
        }
    }

protected:
    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_create_table(qpl_huffman_table_t& huffman_table, qpl_huffman_table_type_e type_c);

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_create_tables(qpl_huffman_table_type_e type_c, qpl_huffman_table_type_e type_d);

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_init_table(qpl_huffman_table_t& huffman_table);

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_init_tables();

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_serialize_table(qpl_huffman_table_t& huffman_table, qpl_serialization_format_e format);

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_serialize_tables(qpl_serialization_format_e format);

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_init_table_with_triplets(qpl_huffman_table_t& huffman_table);

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_init_tables_with_triplets();

    testing::AssertionResult run_init_d_table_with_c_table();

    testing::AssertionResult run_init_c_table_with_d_table();

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_compression();

    template <compression_algorithm_e algorithm>
    testing::AssertionResult run_decompression();

    qpl_huffman_table_t m_d_huffman_table {}; //NOLINT(misc-non-private-member-variables-in-classes)
    qpl_huffman_table_t m_c_huffman_table {}; //NOLINT(misc-non-private-member-variables-in-classes)

private:
    std::vector<uint8_t>                  m_compressed_data {};
    uint32_t                              m_last_bit_offset {};
    std::array<qpl_huffman_triplet, 256U> m_triplets {};
};

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_create_table(qpl_huffman_table_t&     huffman_table,
                                                                       qpl_huffman_table_type_e type) {
    auto status = QPL_STS_OK;

    if constexpr (algorithm == compression_algorithm_huffman_only) {
        status = qpl_huffman_only_table_create(type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &huffman_table);
    }

    if constexpr (algorithm == compression_algorithm_canned || algorithm == compression_algorithm_deflate) {
        status = qpl_deflate_huffman_table_create(type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &huffman_table);
    }

    return (status == QPL_STS_OK) ? testing::AssertionSuccess()
                                  : testing::AssertionFailure() << "Can't create table. Status = " << status;
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_create_tables(qpl_huffman_table_type_e type_c,
                                                                        qpl_huffman_table_type_e type_d) {
    if (run_create_table<algorithm>(m_c_huffman_table, type_c) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't create Compression table";
    }

    if (run_create_table<algorithm>(m_d_huffman_table, type_d) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't create decompression table";
    }

    return testing::AssertionSuccess();
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_init_table(qpl_huffman_table_t& huffman_table) {
    auto status = QPL_STS_OK;

    qpl_histogram histogram {};

    status = qpl_gather_deflate_statistics(source.data(), source.size(), &histogram, qpl_default_level,
                                           GetExecutionPath());
    if (status) { testing::AssertionFailure() << "Can't collect statistics. Status = " << status; }

    if constexpr (algorithm == compression_algorithm_huffman_only) {
        status = qpl_huffman_table_init_with_histogram(huffman_table, &histogram);
    }

    if constexpr (algorithm == compression_algorithm_canned || algorithm == compression_algorithm_deflate) {
        status = qpl_huffman_table_init_with_histogram(huffman_table, &histogram);
    }

    return (status == QPL_STS_OK) ? testing::AssertionSuccess()
                                  : testing::AssertionFailure() << "Can't init table. Status = " << status;
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_serialize_table(qpl_huffman_table_t&       huffman_table,
                                                                          qpl_serialization_format_e format) {
    auto status = QPL_STS_OK;

    serialization_options_t options;
    options.format = format;

    size_t serialized_size = 0U;

    status = qpl_huffman_table_get_serialized_size(huffman_table, options, &serialized_size);
    if (status != QPL_STS_OK) return testing::AssertionFailure() << "Can't get serialized size. Status = " << status;

    auto buffer = std::make_unique<uint8_t[]>(serialized_size + 1U);

    const uint8_t number_to_check = 42U;
    buffer.get()[serialized_size] = number_to_check;

    status = qpl_huffman_table_serialize(huffman_table, buffer.get(), serialized_size, options);
    if (status != QPL_STS_OK) return testing::AssertionFailure() << "Can't serialize table. Status = " << status;

    // performing simple check that we don't overwrite buffer
    if (buffer.get()[serialized_size] != number_to_check) {
        return testing::AssertionFailure() << "Buffer was overwritten during serialization.";
    }

    qpl_huffman_table_t other_huffman_table = nullptr;

    status = qpl_huffman_table_deserialize(buffer.get(), serialized_size, DEFAULT_ALLOCATOR_C, &other_huffman_table);
    if (status != QPL_STS_OK) return testing::AssertionFailure() << "Can't deserialize table. Status = " << status;

    bool are_tables_equal = false;

    status = qpl_huffman_table_compare(huffman_table, other_huffman_table, &are_tables_equal);
    if (status != QPL_STS_OK) {
        qpl_huffman_table_destroy(other_huffman_table);

        return testing::AssertionFailure() << "Error during Huffman tables comparison. Status = " << status;
    }

    status = qpl_huffman_table_destroy(other_huffman_table);
    if (status != QPL_STS_OK) { return testing::AssertionFailure() << "Can't destroy table. Status = " << status; }

    if (!are_tables_equal) return testing::AssertionFailure() << "Tables are not equal. Status = " << status;

    return testing::AssertionSuccess();
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_init_tables() {
    if (run_init_table<algorithm>(m_c_huffman_table) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't init Compression table";
    }

    if (run_init_table<algorithm>(m_d_huffman_table) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't init Decompression table";
    }

    return testing::AssertionSuccess();
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_serialize_tables(qpl_serialization_format_e format) {
    if (run_serialize_table<algorithm>(m_c_huffman_table, format) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't serialize Compression table";
    }

    if (run_serialize_table<algorithm>(m_d_huffman_table, format) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't serialize Decompression table";
    }

    return testing::AssertionSuccess();
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_init_table_with_triplets(qpl_huffman_table_t& huffman_table) {
    return testing::AssertionFailure() << "to implement";
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_init_tables_with_triplets() {
    if (run_init_table_with_triplets<algorithm>(m_c_huffman_table) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't init Compression table with triplets";
    }

    if (run_init_table_with_triplets<algorithm>(m_d_huffman_table) != testing::AssertionSuccess()) {
        return testing::AssertionFailure() << "Can't init Decompression table with triplets";
    }

    return testing::AssertionSuccess();
}

testing::AssertionResult HuffmanTableAlgorithmicTest::run_init_d_table_with_c_table() {
    auto status = qpl_huffman_table_init_with_other(m_d_huffman_table, m_c_huffman_table);

    return (status == QPL_STS_OK) ? testing::AssertionSuccess()
                                  : testing::AssertionFailure()
                                            << "Can't init Decompression table with Compression table . Status = "
                                            << status;
}

testing::AssertionResult HuffmanTableAlgorithmicTest::run_init_c_table_with_d_table() {
    auto status = qpl_huffman_table_init_with_other(m_c_huffman_table, m_d_huffman_table);

    return (status == QPL_STS_OK) ? testing::AssertionSuccess()
                                  : testing::AssertionFailure()
                                            << "Can't init compression table with decompression table. Status = "
                                            << status;
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_compression() {
    auto status = QPL_STS_OK;

    job_ptr->op            = qpl_op_compress;
    job_ptr->level         = qpl_default_level;
    job_ptr->next_in_ptr   = source.data();
    job_ptr->next_out_ptr  = m_compressed_data.data();
    job_ptr->available_in  = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(m_compressed_data.size());
    job_ptr->huffman_table = m_c_huffman_table;

    if constexpr (algorithm == compression_algorithm_huffman_only) {
        job_ptr->flags =
                QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_NO_HDRS | QPL_FLAG_GEN_LITERALS | QPL_FLAG_OMIT_VERIFY;

        status = run_job_api(job_ptr);

        m_last_bit_offset = job_ptr->last_bit_offset;
    }

    if constexpr (algorithm == compression_algorithm_canned) {
        job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

        status = run_job_api(job_ptr);
    }

    if constexpr (algorithm == compression_algorithm_deflate) {
        job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;

        status = run_job_api(job_ptr);
    }

    if (status) { return testing::AssertionFailure() << "Compression Failed. Status = " << status; }

    m_compressed_data.resize(job_ptr->total_out);

    return testing::AssertionSuccess();
}

template <compression_algorithm_e algorithm>
testing::AssertionResult HuffmanTableAlgorithmicTest::run_decompression() {
    auto status = QPL_STS_OK;

    job_ptr->op            = qpl_op_decompress;
    job_ptr->next_in_ptr   = m_compressed_data.data();
    job_ptr->available_in  = static_cast<uint32_t>(m_compressed_data.size());
    job_ptr->next_out_ptr  = destination.data();
    job_ptr->available_out = static_cast<uint32_t>(destination.size());
    ;
    job_ptr->huffman_table = m_d_huffman_table;

    if constexpr (algorithm == compression_algorithm_huffman_only) {
        job_ptr->ignore_end_bits = (8 - m_last_bit_offset) & 7;
        job_ptr->flags           = QPL_FLAG_NO_HDRS | QPL_FLAG_FIRST | QPL_FLAG_LAST;
    }

    if constexpr (algorithm == compression_algorithm_canned) {
        job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
    }

    if constexpr (algorithm == compression_algorithm_deflate) {
        job_ptr->huffman_table = nullptr;
        job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    }

    // Decompress
    status = run_job_api(job_ptr);

    if (status) { return testing::AssertionFailure() << "Decompression Failed. Status = " << status; }

    if (source != destination) { return testing::AssertionFailure() << "Streams are different"; }

    return testing::AssertionSuccess();
}

// Initialization of Huffman table with histogram
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(huffman_table, init_with_histogram, HuffmanTableAlgorithmicTest) {
    auto test_case = GetTestCase();

    // in this test: m_c_huffman_table is compression or combined
    //               m_d_huffman_table is decompression or combined
    QPL_SKIP_TC_TEST(test_case.d_type == decompression_table_type,
                     "initialization from histogram is not supported for decompression table type currently");

    QPL_SKIP_TC_TEST(
            (test_case.c_type == combined_table_type || test_case.d_type == combined_table_type) &&
                    test_case.algorithm == compression_algorithm_huffman_only,
            "initialization from histogram is not supported for combined table type currently with huffman only");

    QPL_SKIP_TC_TEST((test_case.options.format != serialization_raw), "serialization only supports raw format");

    switch (test_case.algorithm) {
        case compression_algorithm_deflate:
            ASSERT_TRUE(run_create_tables<compression_algorithm_deflate>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_tables<compression_algorithm_deflate>());
            ASSERT_TRUE(run_serialize_tables<compression_algorithm_deflate>(test_case.options.format));
            ASSERT_TRUE(run_compression<compression_algorithm_deflate>());
            ASSERT_TRUE(run_decompression<compression_algorithm_deflate>());
            break;
        case compression_algorithm_huffman_only:
            ASSERT_TRUE(run_create_tables<compression_algorithm_huffman_only>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_tables<compression_algorithm_huffman_only>());
            ASSERT_TRUE(run_compression<compression_algorithm_huffman_only>());
            ASSERT_TRUE(run_decompression<compression_algorithm_huffman_only>());
            break;
        case compression_algorithm_canned:
            ASSERT_TRUE(run_create_tables<compression_algorithm_canned>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_tables<compression_algorithm_canned>());
            ASSERT_TRUE(run_serialize_tables<compression_algorithm_canned>(test_case.options.format));
            ASSERT_TRUE(run_compression<compression_algorithm_canned>());
            ASSERT_TRUE(run_decompression<compression_algorithm_canned>());
            break;
    }

    if (m_c_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_c_huffman_table));
        m_c_huffman_table = nullptr;
    }

    if (m_d_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_d_huffman_table));
        m_d_huffman_table = nullptr;
    }
}

// Initialization of Huffman table with triplets
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(huffman_table, DISABLED_init_with_triplet, HuffmanTableAlgorithmicTest) {
    auto test_case = GetTestCase();

    QPL_SKIP_TC_TEST(test_case.algorithm == compression_algorithm_deflate,
                     "initialization with triplets is not supported for deflate table");

    QPL_SKIP_TC_TEST(test_case.algorithm == compression_algorithm_canned,
                     "initialization with triplets is not supported for canned mode");

    QPL_SKIP_TC_TEST((test_case.options.format != serialization_raw), "serialization only supports raw format");

    switch (test_case.algorithm) {
        case compression_algorithm_deflate:
            ASSERT_TRUE(run_create_tables<compression_algorithm_deflate>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_tables_with_triplets<compression_algorithm_deflate>());
            ASSERT_TRUE(run_serialize_tables<compression_algorithm_deflate>(test_case.options.format));
            ASSERT_TRUE(run_compression<compression_algorithm_deflate>());
            ASSERT_TRUE(run_decompression<compression_algorithm_deflate>());
            break;
        case compression_algorithm_huffman_only:
            ASSERT_TRUE(run_create_tables<compression_algorithm_huffman_only>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_tables_with_triplets<compression_algorithm_huffman_only>());
            ASSERT_TRUE(run_compression<compression_algorithm_huffman_only>());
            ASSERT_TRUE(run_decompression<compression_algorithm_huffman_only>());
            break;
        case compression_algorithm_canned:
            ASSERT_TRUE(run_create_tables<compression_algorithm_canned>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_tables_with_triplets<compression_algorithm_canned>());
            ASSERT_TRUE(run_serialize_tables<compression_algorithm_canned>(test_case.options.format));
            ASSERT_TRUE(run_compression<compression_algorithm_canned>());
            ASSERT_TRUE(run_decompression<compression_algorithm_canned>());
            break;
    }

    if (m_c_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_c_huffman_table));
        m_c_huffman_table = nullptr;
    }

    if (m_d_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_d_huffman_table));
        m_d_huffman_table = nullptr;
    }
}

// Initialization of one Huffman table with other Huffman table
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(huffman_table, init_with_other, HuffmanTableAlgorithmicTest) {
    auto test_case = GetTestCase();

    // in this test we first initialize m_c_huffman_table from a histogram and then m_d_huffman_table from it
    QPL_SKIP_TC_TEST(
            test_case.c_type == combined_table_type && test_case.algorithm == compression_algorithm_huffman_only,
            "initialization of huffman only combined table type with histogram is not supported currently");

    // in this test: m_c_huffman_table is compression or combined
    //               m_d_huffman_table is decompression or combined
    QPL_SKIP_TC_TEST(test_case.d_type == combined_table_type,
                     "initialization of combined table type from other table is not supported currently");

    QPL_SKIP_TC_TEST((test_case.options.format != serialization_raw), "serialization only supports raw format");

    switch (test_case.algorithm) {
        case compression_algorithm_deflate:
            ASSERT_TRUE(run_create_tables<compression_algorithm_deflate>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_table<compression_algorithm_deflate>(m_c_huffman_table));
            ASSERT_TRUE(run_init_d_table_with_c_table());
            ASSERT_TRUE(run_serialize_tables<compression_algorithm_deflate>(test_case.options.format));
            ASSERT_TRUE(run_compression<compression_algorithm_deflate>());
            ASSERT_TRUE(run_decompression<compression_algorithm_deflate>());
            break;
        case compression_algorithm_huffman_only:
            ASSERT_TRUE(run_create_tables<compression_algorithm_huffman_only>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_table<compression_algorithm_huffman_only>(m_c_huffman_table));
            ASSERT_TRUE(run_init_d_table_with_c_table());
            ASSERT_TRUE(run_compression<compression_algorithm_huffman_only>());
            ASSERT_TRUE(run_decompression<compression_algorithm_huffman_only>());
            break;
        case compression_algorithm_canned:
            ASSERT_TRUE(run_create_tables<compression_algorithm_canned>(test_case.c_type, test_case.d_type));
            ASSERT_TRUE(run_init_table<compression_algorithm_canned>(m_c_huffman_table));
            ASSERT_TRUE(run_init_d_table_with_c_table());
            ASSERT_TRUE(run_serialize_tables<compression_algorithm_canned>(test_case.options.format));
            ASSERT_TRUE(run_compression<compression_algorithm_canned>());
            ASSERT_TRUE(run_decompression<compression_algorithm_canned>());
            break;
    }

    if (m_c_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_c_huffman_table));
        m_c_huffman_table = nullptr;
    }

    if (test_case.d_type == decompression_table_type) {
        if (m_d_huffman_table) {
            ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_d_huffman_table));
            m_d_huffman_table = nullptr;
        }
    }
    // in this test: m_c_huffman_table is compression or combined
    //               m_d_huffman_table is decompression or combined
    QPL_SKIP_TC_TEST(test_case.d_type == decompression_table_type,
                     "initialization of compression table type from decompression is not supported currently");

    switch (test_case.algorithm) {
        case compression_algorithm_deflate:
            ASSERT_TRUE(run_create_table<compression_algorithm_deflate>(m_c_huffman_table, test_case.c_type));
            ;
            ASSERT_TRUE(run_init_c_table_with_d_table());
            ASSERT_TRUE(run_compression<compression_algorithm_deflate>());
            ASSERT_TRUE(run_decompression<compression_algorithm_deflate>());
            break;
        case compression_algorithm_huffman_only:
            ASSERT_TRUE(run_create_table<compression_algorithm_huffman_only>(m_c_huffman_table, test_case.c_type));
            ASSERT_TRUE(run_init_c_table_with_d_table());
            ASSERT_TRUE(run_compression<compression_algorithm_huffman_only>());
            ASSERT_TRUE(run_decompression<compression_algorithm_huffman_only>());
            break;
        case compression_algorithm_canned:
            ASSERT_TRUE(run_create_table<compression_algorithm_canned>(m_c_huffman_table, test_case.c_type));
            ;
            ASSERT_TRUE(run_init_c_table_with_d_table());
            ASSERT_TRUE(run_compression<compression_algorithm_canned>());
            ASSERT_TRUE(run_decompression<compression_algorithm_canned>());
            break;
    }

    if (m_c_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_c_huffman_table));
        m_c_huffman_table = nullptr;
    }

    if (m_d_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_d_huffman_table));
        m_d_huffman_table = nullptr;
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(huffman_table, deflate_table_get_type, HuffmanTableAlgorithmicTest) {
    auto                     test_case = GetTestCase();
    qpl_huffman_table_type_e c_huffman_table_type {};
    qpl_huffman_table_type_e d_huffman_table_type {};

    switch (test_case.algorithm) {
        case compression_algorithm_huffman_only:
            ASSERT_TRUE(run_create_tables<compression_algorithm_huffman_only>(test_case.c_type, test_case.d_type));

            ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_get_type(m_c_huffman_table, &c_huffman_table_type));
            EXPECT_TRUE(c_huffman_table_type == test_case.c_type);

            ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_get_type(m_d_huffman_table, &d_huffman_table_type));
            EXPECT_TRUE(d_huffman_table_type == test_case.d_type);
            break;

        case compression_algorithm_deflate:
        case compression_algorithm_canned:
            ASSERT_TRUE(run_create_tables<compression_algorithm_canned>(test_case.c_type, test_case.d_type));

            ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_get_type(m_c_huffman_table, &c_huffman_table_type));
            EXPECT_TRUE(c_huffman_table_type == test_case.c_type);

            ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_get_type(m_d_huffman_table, &d_huffman_table_type));
            EXPECT_TRUE(d_huffman_table_type == test_case.d_type);
            break;
    }

    if (m_c_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_c_huffman_table));
        m_c_huffman_table = nullptr;
    }

    if (m_d_huffman_table) {
        ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(m_d_huffman_table));
        m_d_huffman_table = nullptr;
    }
}

} // namespace qpl::test
