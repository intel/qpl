/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstring>

#include "gendefs.hpp"
#include "igenerator.h"

// tests_common
#include "iaa_features_checks.hpp"
#include "operation_test.hpp"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"

typedef struct qpl_decompression_huffman_table qpl_decompression_huffman_table;

extern "C" qpl_decompression_huffman_table* own_huffman_table_get_decompression_table(const qpl_huffman_table_t table);

namespace qpl::test {
class Inflate : public JobFixture {
public:
    void SetUp() override {
        JobFixture::SetUp();
        auto status = qpl_huffman_only_table_create(decompression_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C,
                                                    &d_huffman_table);
        ASSERT_EQ(status, QPL_STS_OK) << "Table creation failed";
    }

    Inflate() {};
    Inflate(const Inflate&)             = delete;
    Inflate(const Inflate&&)            = delete;
    Inflate& operator=(const Inflate&)  = delete;
    Inflate& operator=(const Inflate&&) = delete;

    ~Inflate() override {
        if (d_huffman_table) {
            qpl_huffman_table_destroy(d_huffman_table);
            d_huffman_table = nullptr;
        }
    }

    testing::AssertionResult RunTestOnDataPreset(std::string& compressed_file_name,
                                                 std::string& decompressed_file_name) {
        SetSourceFromFile(compressed_file_name);

        job_ptr->op    = qpl_op_decompress;
        job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

        auto dataset = util::TestEnvironment::GetInstance().GetCompleteDataset();

        try {
            reference_data = dataset[decompressed_file_name];
        } catch (std::exception& e) {
            std::cout << e.what() << '\n';

            return testing::AssertionFailure();
        }

        const qpl_status status = run_job_api(job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);

        return CompareVectors(destination, reference_data, job_ptr->total_out);
    }

    testing::AssertionResult RunTestOnGeneratedData(TestType test_type) {
        SetSourceFromGenerator(test_type);

        job_ptr->op = qpl_op_decompress;

        uint8_t*         next_in_ptr_save   = job_ptr->next_in_ptr;
        const uint32_t   available_in_save  = job_ptr->available_in;
        const uint32_t   total_in_save      = job_ptr->total_in;
        uint8_t*         next_out_ptr_save  = job_ptr->next_out_ptr;
        const uint32_t   available_out_save = job_ptr->available_out;
        const uint32_t   total_out_save     = job_ptr->total_out;
        const qpl_status ref_status         = QPL_STS_MORE_OUTPUT_NEEDED;

        qpl_status status = run_job_api(job_ptr);

        if (ref_status == status) {
            if (test_type == NO_ERR_HUFFMAN_ONLY) {
                for (uint8_t ign_end_bits = 1; ign_end_bits < 8; ign_end_bits++) {
                    job_ptr->next_in_ptr     = next_in_ptr_save;
                    job_ptr->available_in    = available_in_save;
                    job_ptr->total_in        = total_in_save;
                    job_ptr->next_out_ptr    = next_out_ptr_save;
                    job_ptr->available_out   = available_out_save;
                    job_ptr->total_out       = total_out_save;
                    job_ptr->ignore_end_bits = ign_end_bits;
                    job_ptr->last_bit_offset = ign_end_bits;
                    status                   = run_job_api(job_ptr);
                    if (QPL_STS_OK == status) { break; }
                    if (ref_status != status) { break; }
                }
            }
        }

        EXPECT_EQ(QPL_STS_OK, status);

        return CompareVectors(
                destination, reference_data,
                test_type != NO_ERR_HUFFMAN_ONLY ? job_ptr->total_out : static_cast<uint32_t>(destination.size()));
    }

private:
    void SetSourceFromFile(std::string& file_name) {
        auto dataset = util::TestEnvironment::GetInstance().GetCompleteDataset();

        source = dataset[file_name]; // store to original data to reference vector

        destination.resize(source.size() * 10);

        job_ptr->available_in = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr  = source.data();

        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
    }

    void SetSourceFromGenerator(TestType test_type) {
        std::vector<uint8_t> encoded_data_buffer(0);
        std::vector<uint8_t> decoded_data_buffer(0);

        GenStatus  generator_status = GEN_OK;
        TestFactor test_factor;
        test_factor.seed = GetSeed();
        test_factor.type = test_type;
        if (util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_t::qpl_path_hardware) {
            test_factor.specialTestOptions.is_aecs_format2_expected = are_iaa_gen_2_min_capabilities_present();
        }

        const gz_generator::InflateGenerator data_generator;

        generator_status =
                gz_generator::InflateGenerator::generate(encoded_data_buffer, decoded_data_buffer, test_factor);

        if (NO_ERR_HUFFMAN_ONLY == test_type) {
            job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_NO_HDRS | QPL_FLAG_GEN_LITERALS;

            auto* d_table = own_huffman_table_get_decompression_table(d_huffman_table);
            ASSERT_NE(d_table, nullptr) << "Decompression table is null";

            std::memcpy(d_table, &test_factor.specialTestOptions.decompression_huffman_table,
                        sizeof(test_factor.specialTestOptions.decompression_huffman_table));

            job_ptr->huffman_table = d_huffman_table;
        } else {
            job_ptr->flags = (QPL_FLAG_FIRST | QPL_FLAG_LAST) & ~QPL_FLAG_GZIP_MODE;
        }

        EXPECT_EQ(GEN_OK, generator_status);

        source.resize(encoded_data_buffer.size());
        std::copy(encoded_data_buffer.begin(), encoded_data_buffer.end(), source.begin());

        auto destination_size = decoded_data_buffer.size();

        destination.resize(destination_size);
        reference_data.resize(destination_size);
        std::copy(decoded_data_buffer.begin(), decoded_data_buffer.end(), reference_data.begin());

        job_ptr->next_in_ptr  = source.data();
        job_ptr->available_in = static_cast<uint32_t>(source.size());

        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
    }

    testing::AssertionResult CompareDecompressedStreamToReference() {
        if (destination.size() != reference_data.size()) {
            return testing::AssertionFailure() << "Num of elements after decompression: " << destination.size()
                                               << " , actual size: " << reference_data.size();
        } else {
            for (uint32_t i = 0; i < destination.size(); i++) {
                if (destination[i] != reference_data[i]) {
                    return testing::AssertionFailure() << "Output differs at " << i << " index";
                }
            }
        }

        return testing::AssertionSuccess();
    }

    std::vector<uint8_t> reference_data;
    qpl_huffman_table_t  d_huffman_table {};
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate, small_data, Inflate) {
    const TestType test_type = CANNED_SMALL;

    ASSERT_TRUE(RunTestOnGeneratedData(test_type));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate, large_literal_lengths, Inflate) {
    const TestType test_type = CANNED_LARGE_LL;

    ASSERT_TRUE(RunTestOnGeneratedData(test_type));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate, dynamic_block, Inflate) {
    const TestType test_type = NO_ERR_DYNAMIC_BLOCK;

    ASSERT_TRUE(RunTestOnGeneratedData(test_type));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate, fixed_block, Inflate) {
    const TestType test_type = NO_ERR_FIXED_BLOCK;

    ASSERT_TRUE(RunTestOnGeneratedData(test_type));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate, static_block, Inflate) {
    const TestType test_type = NO_ERR_STORED_BLOCK;

    ASSERT_TRUE(RunTestOnGeneratedData(test_type));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate_huffman_only, generated_data, Inflate) {
    const TestType test_type = NO_ERR_HUFFMAN_ONLY;

    ASSERT_TRUE(RunTestOnGeneratedData(test_type));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate, large_distance, Inflate) {
    std::string compressed_source = "gen_large_dist.def";
    std::string encoded_source    = "gen_large_dist.bin";

    ASSERT_TRUE(RunTestOnDataPreset(compressed_source, encoded_source));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(inflate, all_literal_lengths, Inflate) {
    std::string compressed_source = "gen_all_ll.def";
    std::string encoded_source    = "gen_all_ll.bin";

    ASSERT_TRUE(RunTestOnDataPreset(compressed_source, encoded_source));
}
} // namespace qpl::test
