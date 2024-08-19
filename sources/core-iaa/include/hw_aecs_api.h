/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Hardware Interconnect API (private C API)
 */

/**
 * @brief Contains API and Definitions to work with Intel® In-Memory Analytics Accelerator (Intel® IAA)
 * AECS structures (Analytic Engine Configuration and State)
 *
 * @defgroup HW_AECS_API AECS API
 * @ingroup HW_PUBLIC_API
 * @{
 */

#include "hw_definitions.h"
#include "qplc_huffman_table.h"
#include "stdbool.h"

#ifndef HW_PATH_HW_AECS_API_H_
#define HW_PATH_HW_AECS_API_H_

#if !defined(HW_PATH_IAA_AECS_API)
#define HW_PATH_IAA_AECS_API(type, name, arg) type HW_STDCALL hw_iaa_aecs_##name arg
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @brief AECS size for operation: Filter. */
#define HW_AECS_FILTER 0x20U

/**
 * @brief AECS size for Decompress operation,
 * when History Buffer is not required (e.g. Decompress with Indexing).
 * @note Size is sufficient to accommodate AECS Format-1 and 2. */
#define HW_AECS_FILTER_AND_DECOMPRESS_WA_HB 0x4E0

/** @brief AECS size for operation: Decompress. */
#define HW_AECS_FILTER_AND_DECOMPRESS 0x1500U

/** @brief Max size of dictionary in Compress AECS. */
#define HW_AECS_MAX_DICTIONARY_SIZE 0x3000U

/** @brief AECS size for operation: Compress (with Huffman Table). */
#define HW_AECS_COMPRESS_WITH_HT 0x620U

/** @brief AECS size for operation: Compress (with Huffman Table and largest possible dictionary). */
#define HW_AECS_COMPRESS_WITH_HT_AND_DICT (HW_AECS_COMPRESS_WITH_HT + HW_AECS_MAX_DICTIONARY_SIZE)

/** @brief Size of hw_iaa_aecs_decompress structure */
#define HW_AECS_DECOMPRESS_STATE 0x1458U

typedef void                              hw_iaa_aecs;                 /**< Common AECS type */
typedef void                              hw_iaa_huffman_codes;        /**< Forward declaration */
typedef qplc_huffman_table_default_format hw_iaa_c_huffman_only_table; /**< Redefinition */
typedef qplc_huffman_table_flat_format    hw_iaa_d_huffman_only_table; /**< Redefinition */

/**
 * @brief Describe huffman code according to accelerator format
 */
typedef struct {
    uint16_t code;            /**< Huffman code */
    uint8_t  extra_bit_count; /**< Number of extra bits */
    uint8_t  length;          /**< Huffman code length */
} hw_huffman_code;

/**
 * @brief Enumerates all possible access modes for AECS.
 */
typedef enum {
    /**
     * @brief Toggle AECS R/W policy. Default one read from the first AECS and write to the second AECS.
     * If no write then this policy mustn't be used.
     */
    hw_aecs_toggle_rw          = 0x001U,
    hw_aecs_access_read        = 0x010U, /**< Enable reading of the AECS by operation */
    hw_aecs_access_write       = 0x100U, /**< Enable writing to the AECS by operation */
    hw_aecs_access_maybe_write = 0x200U, /**< Enable writing to the AECS only in case of output buffer overflow */
} hw_iaa_aecs_access_policy;

/**
* @brief @todo add description
*/
typedef enum {
    hw_aecs_at_ll_token_non_final_block     = 0x0U, /**< @todo add description */
    hw_aecs_at_ll_token_final_block         = 0x4U, /**< @todo add description */
    hw_aecs_at_stored_block_non_final_block = 0x2U, /**< @todo add description */
    hw_aecs_at_stored_block_final_block     = 0x6U, /**< @todo add description */
    hw_aecs_at_start_block_header           = 0x1U, /**< @todo add description */
    hw_aecs_processing_terminated_with_eob  = 0x8U  /**< @todo add description */
} hw_iaa_aecs_decompress_state;

/**
 * @brief Describes @ref hw_iaa_aecs_analytic substructure that contains additional input/output data of filter operations
 */
typedef struct {
    uint32_t crc;                               /**< Initial (on input) or final (on output) CRC Checksum value */
    uint32_t xor_checksum;                      /**< Initial (on input) or final (on output) Xor Checksum value */
    uint32_t filter_low;                        /**< Low parameter value (low limit) of filter operations */
    uint32_t filter_high;                       /**< High parameter value (high limit) of filter operations */
    uint32_t output_mod_idx;                    /**< Number bytes that should be dropped before Analytic start */
    uint32_t drop_initial_decompress_out_bytes; /**< Number bytes that should be dropped before Filtering start */
} hw_iaa_aecs_filter;

/**
 * @brief Describes @ref hw_iaa_aecs_decompress structure.
 * Contains Decompress part of AECS data, with unions for using either Format-1 or Format-2.
 */
typedef struct {
    // Decompression buffers
    uint32_t output_accumulator[2]; /**< Output Accumulator Data */
    uint32_t output_acc_bits_valid; /**< Output Accumulator Bits Valid, bit 0 is used to indicate AECS Format */
    uint32_t idx_bit_offset;        /**< Bit Offset for Indexing */
    uint64_t input_accum[32];       /**< Input Accumulator Data */
    uint8_t  input_accum_size[32];  /**< Input Accumulator Valid Bits */

    // Decompress/Analytics Internal State
    uint32_t eob_cam_entry;     /**< EOB CAM Entry */
    uint8_t  drop_initial_bits; /**< Drop Initial Bits */
    uint8_t  aecs_format;       /**< AECS Format identifier: 0 for Format-1 and 1 for Format-2 */
    uint16_t reserved0;         /**< Reserved bytes */

    uint32_t lit_len_first_tbl_idx
            [5]; /**< ALU First Table Index, each ALU field is 5 DWORDS or Reserved in case of AECS format-2 */
    uint32_t lit_len_num_codes[5];      /**< ALU Num Codes */
    uint32_t lit_len_first_code[5];     /**< ALU First Code */
    uint32_t lit_len_first_len_code[5]; /**< ALU First Len Code */

    uint32_t reserved1[62]; /**< Reserved bytes, LL CAM, Reserved fields, Distance CAM and Min Length-Code Length */

    union {
        struct {
            uint32_t reserved2[6];          /**< Reserved bytes, as not in use for AECS format-2 */
            uint8_t  ll_mapping_cam_1[224]; /**< LL Mapping CAM, part 1 */
            uint32_t reserved3[5];          /**< Reserved bytes, as not in use for AECS format-2 */
        };
        uint8_t lit_len_sym[268]; /**< LL Mapping Table */
    };

    uint16_t decompress_state; /**< State of decompress parser in bits 3:0 */
    uint16_t reserved4;        /**< Reserved bytes, Number of bits not processed */

    union {
        struct {
            uint32_t reserved5[2];          /**< Reserved bytes, Stored Block Bytes Remaining and Reserved Fields  */
            uint8_t  ll_mapping_cam_2[144]; /**< LL Mapping CAM, part 2 */
            uint32_t reserved6[5];          /**< Reserved bytes */
        };
        uint32_t reserved7[43]; /**< Reserved bytes, Stored Block Bytes Remaining and Reserved Fields */
    };

    struct {
        uint16_t history_buffer_write_offset : 15; /**< Offset to the first unwritten byte in history_buffer */
        uint16_t is_history_buffer_overflowed : 1; /**< True or False, indicates if History Buffer capacity exceeded */
    } history_buffer_params;                       /**< History Buffer Write Pointer Data */
    uint16_t reserved8;                            /**< Reserved bytes */
    uint8_t  history_buffer[4096];                 /**< History Buffer Data */
    uint32_t reserved9[6];                         /**< Reserved bytes, Padding */
} hw_iaa_aecs_decompress;

/**
 * @brief Describes an AECS state shared with Decompress and Filter operations. Decompress and Filter operations
 * constitute a specific operations group called Analytics.
 */
typedef struct {
    hw_iaa_aecs_filter     filtering_options; /**< Contains filter specific data */
    uint32_t               reserved[36];      /**< Reserved bytes */
    hw_iaa_aecs_decompress inflate_options;   /**< Contains decompressor specific data */
} hw_iaa_aecs_analytic;

/**
 * @brief Contains the number of uses for each `length`, `match` and `offset` symbol/code
 */
typedef struct {
    uint32_t ll_sym[286];  /**< LL huffman table */
    uint32_t reserved1[2]; /**< Reserved bytes */
    uint32_t d_sym[30];    /**< D huffman table */
    uint32_t reserved2[2]; /**< Reserved bytes */
} hw_iaa_histogram;

/**
 * @brief Describes an AECS state for Compress operation.
 */
typedef struct {
    uint32_t         crc;                   /**< Initial (on input) or final (on output) CRC Checksum value */
    uint32_t         xor_checksum;          /**< Initial (on input) or final (on output) XOR Checksum value */
    uint32_t         reserved0[5];          /**< Reserved bytes */
    uint32_t         num_output_accum_bits; /**< Number of bits that are valid in Output Accumulator */
    uint8_t          output_accum[256];     /**< Output Accumulator */
    hw_iaa_histogram histogram;             /**< Huffman codes used for compression */
    uint8_t          dictionary[HW_AECS_MAX_DICTIONARY_SIZE]; /**< Dictionary used for compression */
} hw_iaa_aecs_compress;

/* ====== AECS Compress ====== */

/**
 * @name AECS Deflate Compress Mode API
 *
 * @brief Contains AECS service functions to compress stream in the `Deflate` format.
 *
 * @{
 */

/**
 * @brief Specifies deflate fixed header that will be used to declare next compressed block.
 *
 * @param [in,out] aecs_ptr     pointer to @ref hw_iaa_aecs_compress
 * @param [in] b_final           final block marker
 *
 * @note`b_final` must contain `1` or `0` value
 *
 * @return Error in case of aecs corruption.
 */
HW_PATH_IAA_AECS_API(uint32_t, compress_write_deflate_fixed_header,
                     (hw_iaa_aecs_compress* const aecs_ptr, const uint32_t b_final));

/**
 * @brief Specifies deflate dynamic header that will be used to declare next compressed block.
 *
 * @param [in,out] aecs_ptr      pointer to @ref hw_iaa_aecs_compress
 * @param [in] header_ptr        pointer to already prepared deflate header
 * @param [in] header_bit_size   size of header in bits
 * @param [in] b_final           final block marker
 *
 * @note `b_final` must contain `1` or `0` value
 *
 * @return Error in case of aecs corruption.
 *
 */
HW_PATH_IAA_AECS_API(uint32_t, compress_write_deflate_dynamic_header,
                     (hw_iaa_aecs_compress* const aecs_ptr, const uint8_t* const header_ptr,
                      const uint32_t header_bit_size, const uint32_t b_final));

/**
 * @brief Setup @ref hw_iaa_aecs_compress to compress data with previously calculated huffman codes.
 *
 * @param [in,out] aecs_ptr              pointer to @ref hw_iaa_aecs_compress
 * @param [in] literal_length_codes_ptr  pointer to literals and matches huffman codes table
 * @param [in] distance_codes_ptr        pointer to offset huffman codes table
 *
 */
HW_PATH_IAA_AECS_API(void, compress_set_deflate_huffman_table,
                     (hw_iaa_aecs_compress* const aecs_ptr, const hw_iaa_huffman_codes* const literal_length_codes_ptr,
                      const hw_iaa_huffman_codes* const distance_codes_ptr));

/**
 * @brief Specifies deflate dynamic header that will be used to declare next compressed block.
 *
 * @param [in,out] aecs_ptr   pointer to @ref hw_iaa_aecs_compress
 * @param [in] histogram_ptr  histogram collected with using @ref hw_iaa_descriptor_init_statistic_collector
 * @param [in] b_final        final block marker
 *
 */
HW_PATH_IAA_AECS_API(void, compress_write_deflate_dynamic_header_from_histogram,
                     (hw_iaa_aecs_compress* const aecs_ptr, hw_iaa_histogram* const histogram_ptr,
                      const uint32_t b_final));
/** @} */

/**
 * @name AECS Huffman Only Compress Mode API
 *
 * @brief Contains AECS service functions to compress stream in the `Huffman Only` format
 *
 * @{
 */

/**
 * @brief Setup @ref hw_iaa_aecs_compress to compress data with previously calculated huffman codes.
 *
 * @param [in,out] aecs_ptr              pointer to @ref hw_iaa_aecs_compress
 * @param [in] literal_length_codes_ptr  pointer to literals and matches huffman codes table
 *
 */
HW_PATH_IAA_AECS_API(void, compress_set_huffman_only_huffman_table,
                     (hw_iaa_aecs_compress* const aecs_ptr, hw_iaa_huffman_codes* const literal_length_codes_ptr));

/**
 * @brief Setup @ref hw_iaa_aecs_compress to compress data with huffman codes calculated with provided histogram.
 *
 * @param [in,out] aecs_ptr   pointer to @ref hw_iaa_aecs_compress
 * @param [in] histogram_ptr  histogram collected using @ref hw_iaa_descriptor_init_statistic_collector
 *
 */
HW_PATH_IAA_AECS_API(void, compress_set_huffman_only_huffman_table_from_histogram,
                     (hw_iaa_aecs_compress* const aecs_ptr, hw_iaa_histogram* const histogram_ptr));

/**
 * @brief Extract huffman codes table from @ref hw_iaa_aecs_compress and store this into @ref hw_iaa_c_huffman_only_table
 *
 * @param [in] aecs_ptr            pointer to valid @ref hw_iaa_aecs_compress
 * @param [out] huffman_table_ptr  pointer to target huffman table
 *
 */
HW_PATH_IAA_AECS_API(void, compress_store_huffman_only_huffman_table,
                     (const hw_iaa_aecs_compress* const  aecs_ptr,
                      hw_iaa_c_huffman_only_table* const huffman_table_ptr));
/** @} */

/**
 * @name AECS Compress Service API
 *
 * @brief Contains AECS service functions to compress stream in the `Deflate` format
 *
 * @{
 */

/**
 * @brief Clean compress output accumulator.
 *
 * @param [in, out] aecs_ptr pointer to valid @ref hw_iaa_aecs_compress
 *
 */
static inline HW_PATH_IAA_AECS_API(void, compress_clean_accumulator, (hw_iaa_aecs_compress* const aecs_ptr)) {
    aecs_ptr->num_output_accum_bits = 0U;
}

/**
 * @brief Flush N output accumulator bits into specified destination.
 * @param [in, out] aecs_ptr        pointer to valid @ref hw_iaa_aecs_compress
 * @param [out]     next_out_pptr   destination
 * @param [in]      bits            bits to flush
 *
 * @note Output accumulator will be cleaned
 *
 */
HW_PATH_IAA_AECS_API(void, compress_accumulator_flush,
                     (hw_iaa_aecs_compress* const aecs_ptr, uint8_t** const next_out_pptr, const uint32_t bits));

/**
 * @brief Get actual bits count in the output accumulator.
 * @param [in] aecs_ptr pointer to valid @ref hw_iaa_aecs_compress
 *
 * @return actual bits count
 *
 */
static inline HW_PATH_IAA_AECS_API(uint32_t, compress_accumulator_get_actual_bits,
                                   (const hw_iaa_aecs_compress* const aecs_ptr)) {
    return aecs_ptr->num_output_accum_bits;
}

/**
 * @brief Get crc32 and xor checksums values from @ref hw_iaa_aecs_compress.
 *
 * @param [in] aecs_ptr       pointer to valid @ref hw_iaa_aecs_compress
 * @param [out] crc           pointer to save crc32 value
 * @param [out] xor_checksum  pointer to save xor checksum value
 *
 */
static inline HW_PATH_IAA_AECS_API(void, compress_get_checksums,
                                   (const hw_iaa_aecs_compress* const aecs_ptr, uint32_t* const crc,
                                    uint32_t* const xor_checksum)) {
    *xor_checksum = aecs_ptr->xor_checksum;
    *crc          = aecs_ptr->crc;
}

/**
 * @brief Get the address of @ref hw_iaa_aecs_compress with the specified AECS read index.
 *
 * @param [in] ccfg_base   pointer to the base address of @ref hw_iaa_aecs_compress
 * @param [in] aecs_index  AECS read index
 * @param [in] aecs_size   actual size of @ref hw_iaa_aecs_compress, which includes the dictionary
 * @return the address corresponding to the AECS read index provided
 */
static inline HW_PATH_IAA_AECS_API(hw_iaa_aecs_compress*, compress_get_aecs_ptr,
                                   (hw_iaa_aecs_compress* const ccfg_base, const uint32_t aecs_index,
                                    const uint32_t aecs_size)) {
    if ((aecs_index != 0U && aecs_index != 1U) || (aecs_size > HW_AECS_COMPRESS_WITH_HT_AND_DICT)) { return NULL; }

    if (aecs_index == 0U) { return ccfg_base; }

    uint8_t* ccfg = (uint8_t*)(ccfg_base);
    return (hw_iaa_aecs_compress*)(ccfg + aecs_size);
}

/**
 * @brief Set crc32 and xor checksums seeds into @ref hw_iaa_aecs_compress.
 *
 * @param [out] aecs_ptr pointer to @ref hw_iaa_aecs_compress
 * @param [in] crc           crc32 seed
 * @param [in] xor_checksum  xor checksum seed
 *
 */
static inline HW_PATH_IAA_AECS_API(void, compress_set_checksums,
                                   (hw_iaa_aecs_compress* const aecs_ptr, const uint32_t crc,
                                    const uint32_t xor_checksum)) {
    aecs_ptr->xor_checksum = xor_checksum;
    aecs_ptr->crc          = crc;
}

/**
 * @brief Insert `end-of-block` (eob) symbol into output accumulator.
 * @details Can be useful to terminate `Deflate` block.
 *
 * @param [out] eacs_deflate_ptr pointer to valid @ref hw_iaa_aecs_compress
 * @param [in] eob_symbol        `end-of-block` symbol
 */
HW_PATH_IAA_AECS_API(void, compress_accumulator_insert_eob,
                     (hw_iaa_aecs_compress* const eacs_deflate_ptr, const hw_huffman_code eob_symbol));

/**
 * @brief Set dictionary in @ref hw_iaa_aecs_compress
 *
 * @param [out] aecs_ptr                    pointer to valid @ref hw_iaa_aecs_compress
 * @param [in]  dictionary_data_ptr         pointer to dictionary text
 * @param [in]  dictionary_size_in_aecs     AECS dictionary size in bytes
 * @param [in]  aecs_raw_dictionary_offset  offset of raw dictionary in AECS
 */
HW_PATH_IAA_AECS_API(void, compress_set_dictionary,
                     (const hw_iaa_aecs_compress* const aecs_ptr, const uint8_t* const dictionary_data_ptr,
                      const uint32_t dictionary_size_in_aecs, const uint32_t aecs_raw_dictionary_offset));
/** @} */

/* ====== AECS Decompress ====== */

/**
 * @name AECS Huffman Only Decompression API
 *
 * @brief Contains AECS service functions to decompress stream in the `Huffman Only` format
 *
 * @{
 */

/**
 * @brief Setup @ref hw_iaa_aecs_decompress to decompress data with previously calculated huffman codes.
 *
 * @param [out]  aecs_ptr           pointer to @ref hw_iaa_aecs_decompress
 * @param [in]   huffman_table_ptr  pointer to decompression Huffman table
 */
HW_PATH_IAA_AECS_API(void, decompress_set_huffman_only_huffman_table,
                     (hw_iaa_aecs_decompress* const aecs_ptr, hw_iaa_d_huffman_only_table* const huffman_table_ptr));

/**
 * @brief Setup @ref hw_iaa_aecs_decompress to decompress data with huffman codes calculated with provided histogram.
 *
 * @param [in,out] aecs_ptr                 pointer to @ref hw_iaa_aecs_decompress
 * @param [in]     histogram_ptr            histogram collected with using @ref hw_iaa_descriptor_init_statistic_collector
 * @param [in]     is_aecs_format2_expected flag to indicate whether mapping table or CAM should be filled in
 *
 * @return 0 if success, 1 otherwise
 *
 */
HW_PATH_IAA_AECS_API(uint32_t, decompress_set_huffman_only_huffman_table_from_histogram,
                     (hw_iaa_aecs_decompress* const aecs_ptr, const hw_iaa_histogram* const histogram_ptr,
                      bool is_aecs_format2_expected));

/** @} */

/**
 * @name AECS Decompress Service API
 *
 * @brief Contains AECS service functions to compress stream in the `Deflate` format
 *
 * @{
 */

/**
 * @brief Fill decompress input accumulator with initial data
 *
 * @details Can be used to achieve random access to compressed data.
 *
 * @param [out] aecs_ptr          pointer to @ref hw_iaa_aecs_decompress
 * @param [in] source_ptr         data to decompress
 * @param [in] source_size        data size to decompress
 * @param [in] ignore_start_bits  none-actual bit count in the beginning of the source stream
 * @param [in] ignore_end_bits    none-actual bit count in the end of the source stream
 *
 * @note `source_ptr` must be incremented after success function execution.
 * @ref hw_descriptor must be initiated with updated `source_ptr` value
 *
 * @return Error in case of aecs corruption.
 */
HW_PATH_IAA_AECS_API(uint32_t, decompress_set_input_accumulator,
                     (hw_iaa_aecs_decompress* const aecs_ptr, const uint8_t* const source_ptr,
                      const uint32_t source_size, const uint8_t ignore_start_bits, const uint8_t ignore_end_bits));

/**
 * @brief Clean decompress input accumulator by setting all values in input_accum_size to zero.
 *
 * @param [in, out] aecs_ptr pointer to valid @ref hw_iaa_aecs_decompress
 *
 */
HW_PATH_IAA_AECS_API(void, decompress_clean_input_accumulator, (hw_iaa_aecs_decompress* const aecs_ptr));

/**
 * @brief Clean necessary fields in the decompress AECS to avoid using corrupted data.
 *
 * @param [in, out] aecs_ptr pointer to valid @ref hw_iaa_aecs_decompress
 *
 */
HW_PATH_IAA_AECS_API(void, decompress_clean_aecs, (hw_iaa_aecs_decompress* const aecs_ptr));

/**
 * @brief Check if decompress input accumulator is empty.
 *
 * @param [in] aecs_ptr pointer to valid @ref hw_iaa_aecs_decompress
 *
 * @return `true` if empty and `false` in the other case
 */
static inline HW_PATH_IAA_AECS_API(bool, decompress_is_empty_input_accumulator,
                                   (hw_iaa_aecs_decompress* const aecs_ptr)) {
    return 0U == aecs_ptr->input_accum_size[0];
}

/**
 * @brief @todo add description.
 *
 * @param [in, out] aecs_ptr pointer to valid @ref hw_iaa_aecs_decompress
 * @param [in]      decompression_state decompression stage to set
 */
static inline HW_PATH_IAA_AECS_API(void, decompress_set_decompression_state,
                                   (hw_iaa_aecs_decompress* const aecs_ptr,
                                    hw_iaa_aecs_decompress_state  decompression_state)) {
    aecs_ptr->decompress_state = (uint16_t)decompression_state;
}

/**
 * @brief @todo add description
 *
 * @param [in] aecs_ptr                         pointer to valid @ref hw_iaa_aecs_decompress
 * @param [in] raw_dictionary_ptr               pointer to raw dictionary data
 * @param [in] raw_dictionary_size              raw dictionary size
 * @param [in] decompress_dictionary_size       decompression dictionary size
 * @param [in] decompress_raw_dictionary_offset decompression raw dictionary data offset
 */
HW_PATH_IAA_AECS_API(void, decompress_set_dictionary,
                     (hw_iaa_aecs_decompress* const aecs_ptr, const uint8_t* const raw_dictionary_ptr,
                      const size_t raw_dictionary_size, const size_t decompress_dictionary_size,
                      const uint32_t decompress_raw_dictionary_offset));

/** @} */

/* ====== AECS Filter ====== */

/**
 * @name AECS Filter Service API
 *
 * @brief Contains AECS service functions to specify special filtering options
 *
 * @{
 */

/**
 * @brief Set initial output index for analytic operations.
 * @details Makes sense for filtering only.
 *
 * @param [out] aecs_ptr     pointer to @ref hw_iaa_aecs_analytic
 * @param [in] output_index  output index
 *
 */
static inline HW_PATH_IAA_AECS_API(void, filter_set_initial_output_index,
                                   (hw_iaa_aecs_analytic* const aecs_ptr, const uint32_t output_index)) {
    aecs_ptr->filtering_options.output_mod_idx = output_index;
}

/**
 * @brief Set count of bytes to drop for analytic.
 * @details Bytes will be dropped after decompression pass before filtering one.
 *
 * @param [out] aecs_ptr     pointer to @ref hw_iaa_aecs_analytic
 * @param [in] bytes_count   count dropped bytes
 *
 */
static inline HW_PATH_IAA_AECS_API(void, filter_set_drop_initial_decompressed_bytes,
                                   (hw_iaa_aecs_analytic* const aecs_ptr, const uint16_t bytes_count)) {
    aecs_ptr->filtering_options.drop_initial_decompress_out_bytes = bytes_count;
}

/**
 * @brief Set crc32 seed into @ref hw_iaa_aecs_analytic.
 * @details Can be used to specify crc32 seed for analytic (filtering or decompress).
 *
 * @param [out] aecs_ptr pointer to @ref hw_iaa_aecs_analytic
 * @param [in] seed      crc32 seed
 *
 */
static inline HW_PATH_IAA_AECS_API(void, decompress_set_crc_seed,
                                   (hw_iaa_aecs_analytic* const aecs_ptr, const uint32_t seed)) {
    aecs_ptr->filtering_options.crc = seed;
}

/**
 * @brief Set AECS Format related bits to 0 (Format-1) or 1 (Format-2) in @ref hw_iaa_aecs_decompress.
 * @details In order to specify AECS Format two bits are required to be set to 0 or 1:
 * low order bit in the "Output Bits Valid" field and AECS Format Number.
 *
 * @param [out] aecs_ptr pointer to @ref hw_iaa_aecs_decompress
 * @param [in]  is_aecs_format2_expected indicator of AECS Format
 *
 */
static inline HW_PATH_IAA_AECS_API(void, decompress_state_set_aecs_format,
                                   (hw_iaa_aecs_decompress* const aecs_ptr, bool is_aecs_format2_expected)) {
    if (is_aecs_format2_expected) {
        aecs_ptr->output_acc_bits_valid |= 0x1U;
        aecs_ptr->aecs_format |= 1;
    } else {
        aecs_ptr->output_acc_bits_valid &= ~(0x1U);
        aecs_ptr->aecs_format &= ~1;
    }
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif //HW_PATH_HW_AECS_API_H_

/** @} */
