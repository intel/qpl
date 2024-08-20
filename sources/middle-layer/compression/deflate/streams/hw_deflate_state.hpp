/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_MIDDLE_LAYER_COMPRESSION_STREAMS_HW_DEFLATE_STATE_HPP
#define QPL_MIDDLE_LAYER_COMPRESSION_STREAMS_HW_DEFLATE_STATE_HPP

#include <common/linear_allocator.hpp>
#include <compression/multitask/multi_task.hpp>

#include "compression/deflate/compression_units/stored_block_units.hpp"
#include "compression/deflate/deflate.hpp"
#include "compression/deflate/streams/compression_stream.hpp"
#include "compression/dictionary/dictionary_utils.hpp"
#include "hw_aecs_api.h"
#include "hw_definitions.h"
#include "hw_descriptors_api.h"
#include "hw_iaa_flags.h"

namespace qpl::ml::compression {

template <>
class deflate_state<execution_path_t::hardware> final {
    template <execution_path_t path>
    friend class deflate_state_builder;

    template <execution_path_t path, deflate_mode_t mode, class stream_t>
    friend auto deflate(stream_t& stream, uint8_t* begin, const uint32_t size) noexcept
            -> compression_operation_result_t;

    friend auto write_stored_block(deflate_state<execution_path_t::hardware>& state) noexcept
            -> compression_operation_result_t;

    friend class gzip_decorator;

    friend class zlib_decorator;

public:
    [[nodiscard]] static constexpr inline auto get_buffer_size() noexcept -> uint32_t {
        size_t size = 0;

        size += sizeof(meta_data);
        size += util::align_size(sizeof(hw_descriptor)) * 3;
        size += util::align_size(sizeof(hw_completion_record));
        size += util::align_size(sizeof(hw_iaa_aecs_compress)) * 2;
        size += util::align_size(sizeof(hw_iaa_aecs_analytic)) * 2;

        return static_cast<uint32_t>(util::align_size(size, 1_kb));
    }

    static constexpr auto execution_path = execution_path_t::hardware;

protected:
    inline auto allocate_verification_state() noexcept -> void;

    [[nodiscard]] inline auto is_first_chunk() const noexcept -> bool;

    [[nodiscard]] inline auto is_last_chunk() const noexcept -> bool;

    [[nodiscard]] inline auto next_out() const noexcept -> uint8_t*;

    [[nodiscard]] inline auto avail_out() const noexcept -> uint32_t;

    [[nodiscard]] auto crc() const noexcept -> uint32_t;

    inline void set_output_prologue(uint32_t size) noexcept;

    hw_descriptor*                         collect_statistic_descriptor_ = nullptr;
    hw_descriptor*                         compress_descriptor_          = nullptr;
    hw_descriptor*                         verify_descriptor_            = nullptr;
    HW_PATH_VOLATILE hw_completion_record* completion_record_            = nullptr;
    qpl_compression_huffman_table*         huffman_table_                = nullptr;
    qpl_dictionary*                        dictionary_                   = nullptr;
    bool                                   start_new_block               = false;
    util::multitask_status                 processing_step               = util::multitask_status::ready;
    hw_multidescriptor_status              multi_desc_status =
            qpl_none_completed;         /**< Steps that are completed and to be skipped in resubmitted
                                                                                            jobs in case of QPL_STS_QUEUES_ARE_BUSY_ERR */
    uint32_t prev_written_indexes = 0U; // todo align with SW

    /**
     * @brief Meta data for deflate.
     */
    struct meta_data {
        uint8_t               aecs_index  = 0U; /**< AECS read index for deflate AECS */
        uint32_t              stored_bits = 0U; /**< @todo */
        hw_huffman_code       eob_code    = {}; /**< @todo */
        hw_iaa_aecs_compress* aecs_       = nullptr;
        /**< Pointer to the AECS. The memory being referenced is
                                                              twice the size of the AECS. Read and write will occur
                                                              to different halves of the memory, and alternate in case there's
                                                              a sequence of descriptors. The aecs_index and verify_aecs_index
                                                              indicate which half should be read */
        hw_iaa_mini_block_size_t mini_block_size_ = static_cast<hw_iaa_mini_block_size_t>(mini_block_size_none);
        /**< @todo */
        uint32_t prologue_size_    = 0U; /**< @todo */
        uint8_t  verify_aecs_index = 0U; /**< AECS read index for verify AECS */
        uint32_t aecs_size         = HW_AECS_COMPRESS_WITH_HT;
        /**< The actual compress AECS size, which includes any dictionary used.
                                                              This size will be used as the Source 2 transfer size. The default
                                                              value HW_AECS_COMPRESS_WITH_HT is the AECS size without dictionary */
    };

    meta_data* meta_data_ = nullptr;

    // Verify State
    hw_iaa_aecs_analytic* aecs_verify_ = nullptr;

    const util::linear_allocator& allocator_;

    explicit deflate_state(const qpl::ml::util::linear_allocator& allocator, bool init_compress_body)
        : allocator_(allocator) {
        meta_data_           = allocator.allocate<deflate_state<execution_path_t::hardware>::meta_data>();
        compress_descriptor_ = allocator.allocate<hw_descriptor, qpl::ml::util::memory_block_t::aligned_64u>();
        completion_record_   = allocator.allocate<hw_completion_record, qpl::ml::util::memory_block_t::aligned_64u>();
        meta_data_->aecs_    = allocator.allocate<hw_iaa_aecs_compress, qpl::ml::util::memory_block_t::aligned_64u>(2U);

        if (init_compress_body) { hw_iaa_descriptor_init_compress_body(compress_descriptor_); }
    }
};

inline auto deflate_state<execution_path_t::hardware>::is_first_chunk() const noexcept -> bool {
    return processing_step & util::multitask_status::multi_chunk_first_chunk;
}

inline auto deflate_state<execution_path_t::hardware>::is_last_chunk() const noexcept -> bool {
    return processing_step & util::multitask_status::multi_chunk_last_chunk;
}

inline auto deflate_state<execution_path_t::hardware>::next_out() const noexcept -> uint8_t* {
    return ((hw_decompress_analytics_descriptor*)compress_descriptor_)->dst_ptr;
}

inline auto deflate_state<execution_path_t::hardware>::avail_out() const noexcept -> uint32_t {
    return ((hw_decompress_analytics_descriptor*)compress_descriptor_)->max_dst_size;
}

inline void deflate_state<execution_path_t::hardware>::set_output_prologue(uint32_t size) noexcept {
    meta_data_->prologue_size_ = size;
    hw_iaa_descriptor_shift_output_buffer(compress_descriptor_, meta_data_->prologue_size_);
}

inline void deflate_state<execution_path_t::hardware>::allocate_verification_state() noexcept {
    verify_descriptor_ = allocator_.allocate<hw_descriptor, qpl::ml::util::memory_block_t::aligned_64u>();
    aecs_verify_       = allocator_.allocate<hw_iaa_aecs_analytic, qpl::ml::util::memory_block_t::aligned_64u>(2U);

    hw_iaa_descriptor_init_compress_verification(verify_descriptor_);
}

inline auto deflate_state<execution_path_t::hardware>::crc() const noexcept -> uint32_t {
    hw_iaa_aecs_compress* actual_aecs =
            hw_iaa_aecs_compress_get_aecs_ptr(meta_data_->aecs_, meta_data_->aecs_index, meta_data_->aecs_size);
    if (!actual_aecs) { return 0U; }

    return actual_aecs->crc;
}

} // namespace qpl::ml::compression

#endif // QPL_MIDDLE_LAYER_COMPRESSION_STREAMS_HW_DEFLATE_STATE_HPP
