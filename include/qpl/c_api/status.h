/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_STATUS_H_
#define QPL_STATUS_H_

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility push(default)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off

/**
 * @addtogroup JOB_API_DEFINITIONS
 * @{
 */

/**
 * @addtogroup QPL_STATUS_BASE
 * @{
 */

/* --- Status BASE --- */
#define QPL_PROCESSING_ERROR_BASE    0U
        /**< Processing step errors */
#define QPL_PARAMETER_ERROR_BASE     50U
        /**< Parameter check step errors */
#define QPL_SERVICE_LOGIC_ERROR_BASE 100U
        /**< Operation preprocessing or postprocessing errors */
#define QPL_OPERATION_ERROR_BASE     200U
        /**< Execution step errors */
#define QPL_OPERATION_STATUS_BASE    300U
        /**< Execution step statuses */
#define QPL_INIT_ERROR_BASE          500U
        /**< Initialization step errors */

/** @} */

/**
 * @addtogroup QPL_STATUS_CALCULATOR
 * @{
 */

/* --- Status Calculators */
#define QPL_PROCESSING_ERROR(x)     (QPL_PROCESSING_ERROR_BASE + (x))
        /**< Calculates status for processing step */
#define QPL_PARAMETER_ERROR(x)      (QPL_PARAMETER_ERROR_BASE + (x))
        /**< Calculates status for parameter check step */
#define QPL_SERVICE_LOGIC_ERROR(x)  (QPL_SERVICE_LOGIC_ERROR_BASE + (x))
        /**< Calculates status for operation preprocessing or postprocessing step */
#define QPL_OPERATION_ERROR(x)      (QPL_OPERATION_ERROR_BASE + (x))
        /**< Calculates status for operation execution step */
#define QPL_OPERATION_STATUS(x)     (QPL_OPERATION_STATUS_BASE + (x))
        /**< Calculates status for operation execution step */
#define QPL_INIT_ERROR(x)           (QPL_INIT_ERROR_BASE + (x))
        /**< Calculates status for initialization step */

/** @} */

/**
 * @enum qpl_status
 * @brief Intel QPL return status list (Errors marked `Internal` indicate an issue within the library)
 */
typedef enum {
/* ====== Processing Statuses ====== */
    QPL_STS_OK                      = QPL_PROCESSING_ERROR(0),  /**< Operation completed successfully */
    QPL_STS_BEING_PROCESSED         = QPL_PROCESSING_ERROR(1U), /**< The job is still being processed */
    QPL_STS_MORE_OUTPUT_NEEDED      = QPL_PROCESSING_ERROR(2U), /**< Compression/Decompression operation filled output buffer before finishing input */
    QPL_STS_MORE_INPUT_NEEDED       = QPL_PROCESSING_ERROR(3U), /**< Compress/Decompress operation need more input */
    QPL_STS_JOB_NOT_CONTINUABLE_ERR = QPL_PROCESSING_ERROR(4U), /**< A job after a LAST job was not marked as FIRST */
    QPL_STS_QUEUES_ARE_BUSY_ERR     = QPL_PROCESSING_ERROR(5U), /**< Descriptor can't be submitted into filled work queue */
    QPL_STS_LIBRARY_INTERNAL_ERR    = QPL_PROCESSING_ERROR(6U), /**< Unexpected internal error condition */
    QPL_STS_JOB_NOT_SUBMITTED       = QPL_PROCESSING_ERROR(7U), /**< The job being checked/waited has not been submitted */
    QPL_STS_NOT_SUPPORTED_BY_WQ     = QPL_PROCESSING_ERROR(8U), /**< Work queue not configured to support operation */

/* ====== Operations Statuses ====== */
/* --- Incorrect Parameter Value --- */
// <--- Common
    QPL_STS_NULL_PTR_ERR           = QPL_PARAMETER_ERROR(0U), /**< Null pointer error */
    QPL_STS_OPERATION_ERR          = QPL_PARAMETER_ERROR(1U), /**< Non-supported value in the qpl_job operation field */
    QPL_STS_NOT_SUPPORTED_MODE_ERR = QPL_PARAMETER_ERROR(2U), /**< Indicates an error if the requested mode is not supported */
    QPL_STS_BAD_JOB_STRUCT_ERR     = QPL_PARAMETER_ERROR(3U), /**< Indicates that the qpl_job structure does not match the operation */
    QPL_STS_PATH_ERR               = QPL_PARAMETER_ERROR(4U), /**< Incorrect value for the qpl_path input parameter */
    QPL_STS_INVALID_PARAM_ERR      = QPL_PARAMETER_ERROR(5U), /**< Invalid combination of fields in the qpl_job structure */
    QPL_STS_FLAG_CONFLICT_ERR      = QPL_PARAMETER_ERROR(6U), /**< qpl_job flags field contains conflicted values */
    QPL_STS_SIZE_ERR               = QPL_PARAMETER_ERROR(7U), /**< Incorrect size error */
    QPL_STS_BUFFER_TOO_LARGE_ERR   = QPL_PARAMETER_ERROR(8U), /**< Buffer exceeds max size supported by library */
    QPL_STS_BUFFER_OVERLAP_ERR     = QPL_PARAMETER_ERROR(9U), /**< Buffers overlap */

// <-- Simple Operations
    QPL_STS_CRC64_BAD_POLYNOM      = QPL_PARAMETER_ERROR(10U), /**< Incorrect polynomial value for CRC64 */

// <-- Filtering
    QPL_STS_SET_TOO_LARGE_ERR           = QPL_PARAMETER_ERROR(20U), /**< Set is too large for operation */
    QPL_STS_PARSER_ERR                  = QPL_PARAMETER_ERROR(21U), /**< Non-supported value in the qpl_job parser field */
    QPL_STS_OUT_FORMAT_ERR              = QPL_PARAMETER_ERROR(22U), /**< qpl_job out_bit_width field contains invalid value or QPL_FLAG_FORCE_ARRAY_OUTPUT is set with nominal out_bit_width */
    QPL_STS_DROP_BITS_OVERFLOW_ERR      = QPL_PARAMETER_ERROR(23U), /**< Incorrect dropBits value (param_low + param_high must be beyond 0..32) */
    QPL_STS_BIT_WIDTH_OUT_EXTENDED_ERR  = QPL_PARAMETER_ERROR(24U), /**< qpl_job bit-width field contains an invalid value for current output format */
    QPL_STS_DROP_BYTES_ERR              = QPL_PARAMETER_ERROR(25U), /**< qpl_job drop_initial_bytes field contains an invalid value */

// <-- Compression/Decompression
    QPL_STS_MISSING_HUFFMAN_TABLE_ERR     = QPL_PARAMETER_ERROR(30U), /**< Flags specify NO_HDRS and DYNAMIC_HUFFMAN, but no Huffman table provided */
    QPL_STS_INVALID_HUFFMAN_TABLE_ERR     = QPL_PARAMETER_ERROR(31U), /**< Invalid Huffman table data */
    QPL_STS_MISSING_INDEX_TABLE_ERR       = QPL_PARAMETER_ERROR(32U), /**< Indexing enabled but Indexing table is not set */
    QPL_STS_INVALID_COMPRESS_STYLE_ERR    = QPL_PARAMETER_ERROR(33U), /**< The style of a compression job does not match the style of the previous related job */
    QPL_STS_INFLATE_NEED_DICT_ERR         = QPL_PARAMETER_ERROR(34U), /**< Inflate needs dictionary to perform decompression */
    QPL_STS_INVALID_DECOMP_END_PROC_ERR   = QPL_PARAMETER_ERROR(35U), /**< The qpl_job field for decompression manipulation is incorrect */
    QPL_STS_INVALID_BLOCK_SIZE_ERR        = QPL_PARAMETER_ERROR(36U), /**< Invalid block size used during indexing */
    QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL = QPL_PARAMETER_ERROR(37U), /**< Compression level is not supported */
    QPL_STS_HUFFMAN_TABLE_TYPE_ERROR      = QPL_PARAMETER_ERROR(38U), /**< Incorrect table type used */

// <-- Serialization
    QPL_STS_SERIALIZATION_FORMAT_ERROR    = QPL_PARAMETER_ERROR(40U), /**< Unsupported Serialization format */
    QPL_STS_SERIALIZATION_CORRUPTED_DUMP  = QPL_PARAMETER_ERROR(41U), /**< Deserialization can't re-create the table from the input stream */

// <-- Huffman Only BE16 Format Decompression
    QPL_STS_HUFFMAN_BE_IGNORE_MORE_THAN_7_BITS_ERR = QPL_PARAMETER_ERROR(45U), /**< Limitation in IAA 1.0, Huffman only BE16 decompression cannot work if ignore_end_bits is greater than 7 */
    QPL_STS_HUFFMAN_BE_ODD_INPUT_SIZE_ERR          = QPL_PARAMETER_ERROR(46U), /**< Huffman only BE16 decompression input has an odd number of bytes */

/* --- Processing Errors --- */
    QPL_STS_INVALID_DEFLATE_DATA_ERR  = QPL_SERVICE_LOGIC_ERROR(0U), /**< Currently unused */
    QPL_STS_NO_MEM_ERR                = QPL_SERVICE_LOGIC_ERROR(1U), /**< Not enough memory for the operation */
    QPL_STS_INDEX_ARRAY_TOO_SMALL     = QPL_SERVICE_LOGIC_ERROR(2U), /**< Indexing buffer is too small */
    QPL_STS_INDEX_GENERATION_ERR      = QPL_SERVICE_LOGIC_ERROR(3U), /**< Mini-block creation error */
    QPL_STS_ARCHIVE_HEADER_ERR        = QPL_SERVICE_LOGIC_ERROR(4U), /**< Invalid GZIP/Zlib header */
    QPL_STS_ARCHIVE_UNSUP_METHOD_ERR  = QPL_SERVICE_LOGIC_ERROR(5U), /**< Gzip/Zlib header specifies unsupported compress method */
    QPL_STS_OBJECT_ALLOCATION_ERR     = QPL_SERVICE_LOGIC_ERROR(6U), /**< Not able to allocate Huffman table object */

/* --- Corresponds to the error codes in Completion Record from Intel® In-Memory Analytics Accelerator --- */
    QPL_STS_BIG_HEADER_ERR                  = QPL_OPERATION_ERROR(1U),  /**< Reached the end of the input stream before decoding header and header is too big to fit in input buffer */
    QPL_STS_UNDEF_CL_CODE_ERR               = QPL_OPERATION_ERROR(2U),  /**< Bad CL code */
    QPL_STS_FIRST_LL_CODE_16_ERR            = QPL_OPERATION_ERROR(3U),  /**< First code in LL tree is 16 */
    QPL_STS_FIRST_D_CODE_16_ERR             = QPL_OPERATION_ERROR(4U),  /**< First code in D tree is 16 */
    QPL_STS_NO_LL_CODE_ERR                  = QPL_OPERATION_ERROR(5U),  /**< All LL codes are specified with 0 length */
    QPL_STS_WRONG_NUM_LL_CODES_ERR          = QPL_OPERATION_ERROR(6U),  /**< After parsing LL code lengths, total codes != expected value */
    QPL_STS_WRONG_NUM_DIST_CODES_ERR        = QPL_OPERATION_ERROR(7U),  /**< After parsing D code lengths, total codes != expected value */
    QPL_STS_BAD_CL_CODE_LEN_ERR             = QPL_OPERATION_ERROR(8U),  /**< First CL code of length N is greater than 2^N-1 */
    QPL_STS_BAD_LL_CODE_LEN_ERR             = QPL_OPERATION_ERROR(9U),  /**< First LL code of length N is greater than 2^N-1 */
    QPL_STS_BAD_DIST_CODE_LEN_ERR           = QPL_OPERATION_ERROR(10U), /**< First D code of length N is greater than 2^N-1 */
    QPL_STS_BAD_LL_CODE_ERR                 = QPL_OPERATION_ERROR(11U), /**< Incorrect LL code */
    QPL_STS_BAD_D_CODE_ERR                  = QPL_OPERATION_ERROR(12U), /**< Incorrect D code */
    QPL_STS_INVALID_BLOCK_TYPE              = QPL_OPERATION_ERROR(13U), /**< Invalid type of deflate block */
    QPL_STS_INVALID_STORED_LEN_ERR          = QPL_OPERATION_ERROR(14U), /**< Length of stored block doesn't match inverse length */
    QPL_STS_BAD_EOF_ERR                     = QPL_OPERATION_ERROR(15U), /**< EOB flag was set but last token was not EOB */
    QPL_STS_BAD_LEN_ERR                     = QPL_OPERATION_ERROR(16U), /**< Decoded Length code is 0 or greater 258 */
    QPL_STS_BAD_DIST_ERR                    = QPL_OPERATION_ERROR(17U), /**< Decoded Distance is 0 or greater than History Buffer */
    QPL_STS_REF_BEFORE_START_ERR            = QPL_OPERATION_ERROR(18U), /**< Distance of reference is before start of file */
    QPL_STS_TIMEOUT_ERR                     = QPL_OPERATION_ERROR(19U), /**< Library has input data, but is not making forward progress */
    QPL_STS_PRLE_FORMAT_ERR                 = QPL_OPERATION_ERROR(20U), /**< PRLE format is incorrect or is truncated */
    QPL_STS_OUTPUT_OVERFLOW_ERR             = QPL_OPERATION_ERROR(21U), /**< Output index value is greater than max available for current output data type */
    QPL_STS_INTL_AECS_ERR                   = QPL_OPERATION_ERROR(22U), /**< Internal Error Code */
    QPL_STS_SRC1_TOO_SMALL_ERR              = QPL_OPERATION_ERROR(23U), /**< Source 1 contained fewer than expected elements/bytes */
    QPL_STS_SRC2_IS_SHORT_ERR               = QPL_OPERATION_ERROR(24U), /**< Source 2 contained fewer than expected elements/bytes */
    QPL_STS_DST_IS_SHORT_ERR                = QPL_OPERATION_ERROR(25U), /**< qpl_job destination buffer has less bytes than required to process num_input_elements/bytes */
    QPL_STS_INTL_DIST_SPANS_MINI_BLOCKS     = QPL_OPERATION_ERROR(26U), /**< Internal Error Code */
    QPL_STS_INTL_LEN_SPANS_MINI_BLOCKS      = QPL_OPERATION_ERROR(27U), /**< Internal Error Code */
    QPL_STS_INTL_VERIF_INVALID_BLOCK_SIZE   = QPL_OPERATION_ERROR(28U), /**< Internal Error Code */
    QPL_STS_INTL_VERIFY_ERR                 = QPL_OPERATION_ERROR(29U), /**< Internal Error Code */
    QPL_STS_INVALID_HUFFCODE_ERR            = QPL_OPERATION_ERROR(30U), /**< Compressor tried to use an invalid huffman code */
    QPL_STS_BIT_WIDTH_ERR                   = QPL_OPERATION_ERROR(31U), /**< Bit width is out of range [1..32] */
    QPL_STS_SRC_IS_SHORT_ERR                = QPL_OPERATION_ERROR(32U), /**< The input stream ended before specified Number of input Element was seen  */
    QPL_STS_INTL_INVALID_COUNT              = QPL_OPERATION_ERROR(33U), /**< Internal Error Code */
    QPL_STS_TOO_MANY_LL_CODES_ERR           = QPL_OPERATION_ERROR(35U), /**< The number of LL codes specified in the DEFLATE header exceed 286 */
    QPL_STS_TOO_MANY_D_CODES_ERR            = QPL_OPERATION_ERROR(36U), /**< The number of D codes specified in the DEFLATE header exceed 30 */

/* --- Corresponds to the status codes in the Completion Record from Intel® In-Memory Analytics Accelerator --- */
    QPL_STS_INTL_INPROG                 = QPL_OPERATION_STATUS(0U),   /**< Internal Status Code */
    QPL_STS_INTL_SUCCESS                = QPL_OPERATION_STATUS(1U),   /**< Internal Status Code */
    QPL_STS_INTL_PAGE_FAULT             = QPL_OPERATION_STATUS(3U),   /**< Page Fault occurred on Read */
    QPL_STS_INTL_INVALID_PAGE_REQ       = QPL_OPERATION_STATUS(4U),   /**< Internal Status Code */
    QPL_STS_INTL_ANALYTIC_ERROR         = QPL_OPERATION_STATUS(10U),  /**< Internal Status Code */
    QPL_STS_INTL_OUTPUT_OVERFLOW        = QPL_OPERATION_STATUS(11U),  /**< Internal Status Code */
    QPL_STS_INTL_UNSUPPORTED_OPCODE     = QPL_OPERATION_STATUS(16U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_OP_FLAG        = QPL_OPERATION_STATUS(17U),  /**< Internal Status Code */
    QPL_STS_INTL_NONZERO_RESERVED_FIELD = QPL_OPERATION_STATUS(18U),  /**< Internal Status Code */
    QPL_STS_TRANSFER_SIZE_INVALID       = QPL_OPERATION_STATUS(19U),  /**< Invalid value for transfer size or maximum destination size */
    QPL_STS_INTL_OVERLAPPING_BUFFERS    = QPL_OPERATION_STATUS(22U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_COMP_HANDLE    = QPL_OPERATION_STATUS(25U),  /**< Internal Status Code */
    QPL_STS_INTL_TRANSLATION_PAGE_FAULT = QPL_OPERATION_STATUS(26U),  /**< Internal Status Code */
    QPL_STS_INTL_COMPL_RECORD_UNALIGN   = QPL_OPERATION_STATUS(27U),  /**< Internal Status Code */
    QPL_STS_INTL_MISALIGNED_ADDRESS     = QPL_OPERATION_STATUS(28U),  /**< Internal Status Code */
    QPL_STS_INTL_PRIVILIGE_ERROR        = QPL_OPERATION_STATUS(29U),  /**< Internal Status Code */
    QPL_STS_INTL_TRAFFIC_CLASS_ERROR    = QPL_OPERATION_STATUS(30U),  /**< Internal Status Code */
    QPL_STS_INTL_DRAIN_PAGE_FAULT       = QPL_OPERATION_STATUS(31U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_UR_CA_RESPONSE = QPL_OPERATION_STATUS(32U),  /**< Internal Status Code */
    QPL_STS_INTL_READBACK_TIMEOUT       = QPL_OPERATION_STATUS(33U),  /**< Internal Status Code */
    QPL_STS_INTL_HARDWARE_TIMEOUT       = QPL_OPERATION_STATUS(34U),  /**< Internal Status Code */
    QPL_STS_INTL_PAGE_REQUEST_TIMEOUT   = QPL_OPERATION_STATUS(35U),  /**< Internal Status Code */
    QPL_STS_INTL_WATCHDOG_TIMER_EXPIRE  = QPL_OPERATION_STATUS(36U),  /**< Internal Status Code */
    QPL_STS_INTL_PASID_ERROR            = QPL_OPERATION_STATUS(40U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_IDPTE_HANDLE   = QPL_OPERATION_STATUS(41U),  /**< Internal Status Code */
    QPL_STS_INTL_IDTPE_PERMISSION_ERROR = QPL_OPERATION_STATUS(42U),  /**< Internal Status Code */
    QPL_STS_INTL_ID_WINDOW_ERROR        = QPL_OPERATION_STATUS(43U),  /**< Internal Status Code */
    QPL_STS_INTL_UPDATE_INVALID_IDPTE   = QPL_OPERATION_STATUS(44U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_WINDOW_CONTROL = QPL_OPERATION_STATUS(45U),  /**< Internal Status Code */
    QPL_STS_INTL_INACCESSIBLE_DOMAIN    = QPL_OPERATION_STATUS(46U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_DECOMP_FLAG    = QPL_OPERATION_STATUS(48U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_FILTER_FLAG    = QPL_OPERATION_STATUS(49U),  /**< Internal Status Code */
    QPL_STS_INTL_INVALID_INPUT_SIZE     = QPL_OPERATION_STATUS(50U),  /**< Internal Status Code */
    QPL_STS_INVALID_NUM_ELEM            = QPL_OPERATION_STATUS(51U),  /**< Number Elements for Filter operation is 0 */
    QPL_STS_INVALID_SRC1_WIDTH          = QPL_OPERATION_STATUS(52U),  /**< Invalid source-1 bit-width */
    QPL_STS_INV_OUTPUT                  = QPL_OPERATION_STATUS(53U),  /**< Invert Output flag was used when the output was not a bit-vector */
    QPL_STS_INTL_W_PAGE_FAULT           = QPL_OPERATION_STATUS(131U), /**< Page Fault occurred on Write */
    QPL_STS_INTL_W_INVALID_PAGE_REQ     = QPL_OPERATION_STATUS(132U), /**< Internal Status Code */
    QPL_STS_INTL_W_TRANSLATION_PF       = QPL_OPERATION_STATUS(154U), /**< Internal Status Code */

/* ====== Initialization Statuses ====== */
    QPL_STS_INIT_HW_NOT_SUPPORTED             = QPL_INIT_ERROR(0U), /**< Executing using qpl_path_hardware is not supported, check library System Requirements */
    QPL_STS_INIT_LIBACCEL_NOT_FOUND           = QPL_INIT_ERROR(1U), /**< libaccel is not found or not compatible */
    QPL_STS_INIT_LIBACCEL_ERROR               = QPL_INIT_ERROR(2U), /**< libaccel internal error */
    QPL_STS_INIT_WORK_QUEUES_NOT_AVAILABLE    = QPL_INIT_ERROR(3U), /**< Supported and enabled work queues are not found (May be due to lack of privileges e.g. lack of sudo on linux)*/
} qpl_status;

/** @} */

// clang-format on

#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility pop
#endif

#endif //QPL_STATUS_H_
