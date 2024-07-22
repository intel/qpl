/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <assert.h>

#include "hw_aecs_api.h"
#include "own_compress.h"
#include "own_hw_definitions.h"

// c-api
#include "own_checkers.h"

// core-sw
#include "simple_memory_ops_c_bind.h"

#define OWN_INFLATE_INPUT_ACCUMULATOR_DQ_COUNT 32U
#define OWN_MAX_BIT_IDX                        7U

static_assert(sizeof(hw_iaa_aecs_analytic) == HW_AECS_FILTER_AND_DECOMPRESS, "hw_aecs_analytic size is not correct");
static_assert(sizeof(hw_iaa_aecs_decompress) == HW_AECS_DECOMPRESS_STATE, "hw_iaa_aecs_decompress size is not correct");

#define OWN_STATUS_OK    0U
#define OWN_STATUS_ERROR 1U

/**
 * @brief Helper for packing Huffman table with AECS Format-1
 *
 * @param[out]  out  output packed values
 * @param[in]   in   input unpacked values
 *
 */
static inline void pack_table(uint32_t out[5], const uint16_t in[15]) {
    out[0] = ((uint32_t)(in[0] & ((1U << 2U) - 1U)));           // mask = 00000000000000000000000000000011
    out[0] |= ((uint32_t)(in[1] & ((1U << 3U) - 1U))) << 2U;    // mask = 00000000000000000000000000011100
    out[0] |= ((uint32_t)(in[2] & ((1U << 4U) - 1U))) << 5U;    // mask = 00000000000000000000000111100000
    out[0] |= ((uint32_t)(in[3] & ((1U << 5U) - 1U))) << 9U;    // mask = 00000000000000000011111000000000
    out[0] |= ((uint32_t)(in[4] & ((1U << 6U) - 1U))) << 14U;   // mask = 00000000000011111100000000000000
    out[0] |= ((uint32_t)(in[5] & ((1U << 7U) - 1U))) << 20U;   // mask = 00000111111100000000000000000000
    out[1] = ((uint32_t)(in[6] & ((1U << 8U) - 1U)));           // mask = 00000000000000000000000011111111
    out[1] |= ((uint32_t)(in[7] & ((1U << 9U) - 1U))) << 8U;    // mask = 00000000000000011111111100000000
    out[1] |= ((uint32_t)(in[8] & ((1U << 10U) - 1U))) << 17U;  // mask = 00000111111111100000000000000000
    out[2] = ((uint32_t)(in[9] & ((1U << 11U) - 1U)));          // mask = 00000000000000000000011111111111
    out[2] |= ((uint32_t)(in[10] & ((1U << 12U) - 1U))) << 11U; // mask = 00000000011111111111100000000000
    out[3] = ((uint32_t)(in[11] & ((1U << 13U) - 1U)));         // mask = 00000000000000000001111111111111
    out[3] |= ((uint32_t)(in[12] & ((1U << 14U) - 1U))) << 13U; // mask = 00000111111111111110000000000000
    out[4] = ((uint32_t)(in[13] & ((1U << 15U) - 1U)));         // mask = 00000000000000000111111111111111
    out[4] |= ((uint32_t)(in[14] & ((1U << 16U) - 1U))) << 15U; // mask = 01111111111111111000000000000000
}

/**
 * @brief Helper for packing Huffman table with AECS Format-2
 *
 * @param[out]  out_cam_p1  output packed values, first part of LL Mapping Cam
 * @param[out]  out_cam_p2  output packed values, second part of LL Mapping Cam
 * @param[in]   in_cam      input unpacked values from Huffman Table
 *
 * @note Gap occurs between QWORD 27 and 28 (or Byte 223 and 224 due to AECS Format)
 */
static void pack_cam(uint8_t* out_cam_p1, uint8_t* out_cam_p2, uint16_t* in_cam) {

    // Point to first part of the LL Mapping Cam
    uint64_t* tmp_cam_ptr = (uint64_t*)out_cam_p1;

    tmp_cam_ptr[0] = (((uint64_t)(in_cam[0] & 0xF)) << 0) | (((uint64_t)(in_cam[1] & 0x1F)) << 4) |
                     (((uint64_t)(in_cam[2] & 0x3F)) << 9) | (((uint64_t)(in_cam[3] & 0x3F)) << 15) |
                     (((uint64_t)(in_cam[4] & 0x7F)) << 21) | (((uint64_t)(in_cam[5] & 0x7F)) << 28) |
                     (((uint64_t)(in_cam[6] & 0x7F)) << 35) | (((uint64_t)(in_cam[7] & 0x7F)) << 42) |
                     (((uint64_t)(in_cam[8] & 0xFF)) << 49) | (((uint64_t)(in_cam[9] & 0xFF)) << 57);
    tmp_cam_ptr[1] = (((uint64_t)(in_cam[9] & 0xFF)) >> 7) | (((uint64_t)(in_cam[10] & 0xFF)) << 1) |
                     (((uint64_t)(in_cam[11] & 0xFF)) << 9) | (((uint64_t)(in_cam[12] & 0xFF)) << 17) |
                     (((uint64_t)(in_cam[13] & 0xFF)) << 25) | (((uint64_t)(in_cam[14] & 0xFF)) << 33) |
                     (((uint64_t)(in_cam[15] & 0xFF)) << 41) | (((uint64_t)(in_cam[16] & 0x1FF)) << 49) |
                     (((uint64_t)(in_cam[17] & 0x1FF)) << 58);
    tmp_cam_ptr[2] = (((uint64_t)(in_cam[17] & 0x1FF)) >> 6) | (((uint64_t)(in_cam[18] & 0x1FF)) << 3) |
                     (((uint64_t)(in_cam[19] & 0x1FF)) << 12) | (((uint64_t)(in_cam[20] & 0x1FF)) << 21) |
                     (((uint64_t)(in_cam[21] & 0x1FF)) << 30) | (((uint64_t)(in_cam[22] & 0x1FF)) << 39) |
                     (((uint64_t)(in_cam[23] & 0x1FF)) << 48) | (((uint64_t)(in_cam[24] & 0x1FF)) << 57);
    tmp_cam_ptr[3] = (((uint64_t)(in_cam[24] & 0x1FF)) >> 7) | (((uint64_t)(in_cam[25] & 0x1FF)) << 2) |
                     (((uint64_t)(in_cam[26] & 0x1FF)) << 11) | (((uint64_t)(in_cam[27] & 0x1FF)) << 20) |
                     (((uint64_t)(in_cam[28] & 0x1FF)) << 29) | (((uint64_t)(in_cam[29] & 0x1FF)) << 38) |
                     (((uint64_t)(in_cam[30] & 0x1FF)) << 47) | (((uint64_t)(in_cam[31] & 0x1FF)) << 56);
    tmp_cam_ptr[4] = (((uint64_t)(in_cam[31] & 0x1FF)) >> 8) | (((uint64_t)(in_cam[32] & 0x3FF)) << 1) |
                     (((uint64_t)(in_cam[33] & 0x3FF)) << 11) | (((uint64_t)(in_cam[34] & 0x3FF)) << 21) |
                     (((uint64_t)(in_cam[35] & 0x3FF)) << 31) | (((uint64_t)(in_cam[36] & 0x3FF)) << 41) |
                     (((uint64_t)(in_cam[37] & 0x3FF)) << 51) | (((uint64_t)(in_cam[38] & 0x3FF)) << 61);
    tmp_cam_ptr[5] = (((uint64_t)(in_cam[38] & 0x3FF)) >> 3) | (((uint64_t)(in_cam[39] & 0x3FF)) << 7) |
                     (((uint64_t)(in_cam[40] & 0x3FF)) << 17) | (((uint64_t)(in_cam[41] & 0x3FF)) << 27) |
                     (((uint64_t)(in_cam[42] & 0x3FF)) << 37) | (((uint64_t)(in_cam[43] & 0x3FF)) << 47) |
                     (((uint64_t)(in_cam[44] & 0x3FF)) << 57);
    tmp_cam_ptr[6] = (((uint64_t)(in_cam[44] & 0x3FF)) >> 7) | (((uint64_t)(in_cam[45] & 0x3FF)) << 3) |
                     (((uint64_t)(in_cam[46] & 0x3FF)) << 13) | (((uint64_t)(in_cam[47] & 0x3FF)) << 23) |
                     (((uint64_t)(in_cam[48] & 0x3FF)) << 33) | (((uint64_t)(in_cam[49] & 0x3FF)) << 43) |
                     (((uint64_t)(in_cam[50] & 0x3FF)) << 53) | (((uint64_t)(in_cam[51] & 0x3FF)) << 63);
    tmp_cam_ptr[7] = (((uint64_t)(in_cam[51] & 0x3FF)) >> 1) | (((uint64_t)(in_cam[52] & 0x3FF)) << 9) |
                     (((uint64_t)(in_cam[53] & 0x3FF)) << 19) | (((uint64_t)(in_cam[54] & 0x3FF)) << 29) |
                     (((uint64_t)(in_cam[55] & 0x3FF)) << 39) | (((uint64_t)(in_cam[56] & 0x3FF)) << 49) |
                     (((uint64_t)(in_cam[57] & 0x3FF)) << 59);
    tmp_cam_ptr[8] = (((uint64_t)(in_cam[57] & 0x3FF)) >> 5) | (((uint64_t)(in_cam[58] & 0x3FF)) << 5) |
                     (((uint64_t)(in_cam[59] & 0x3FF)) << 15) | (((uint64_t)(in_cam[60] & 0x3FF)) << 25) |
                     (((uint64_t)(in_cam[61] & 0x3FF)) << 35) | (((uint64_t)(in_cam[62] & 0x3FF)) << 45) |
                     (((uint64_t)(in_cam[63] & 0x3FF)) << 55);
    tmp_cam_ptr[9] = (((uint64_t)(in_cam[63] & 0x3FF)) >> 9) | (((uint64_t)(in_cam[64] & 0x7FF)) << 1) |
                     (((uint64_t)(in_cam[65] & 0x7FF)) << 12) | (((uint64_t)(in_cam[66] & 0x7FF)) << 23) |
                     (((uint64_t)(in_cam[67] & 0x7FF)) << 34) | (((uint64_t)(in_cam[68] & 0x7FF)) << 45) |
                     (((uint64_t)(in_cam[69] & 0x7FF)) << 56);
    tmp_cam_ptr[10] = (((uint64_t)(in_cam[69] & 0x7FF)) >> 8) | (((uint64_t)(in_cam[70] & 0x7FF)) << 3) |
                      (((uint64_t)(in_cam[71] & 0x7FF)) << 14) | (((uint64_t)(in_cam[72] & 0x7FF)) << 25) |
                      (((uint64_t)(in_cam[73] & 0x7FF)) << 36) | (((uint64_t)(in_cam[74] & 0x7FF)) << 47) |
                      (((uint64_t)(in_cam[75] & 0x7FF)) << 58);
    tmp_cam_ptr[11] = (((uint64_t)(in_cam[75] & 0x7FF)) >> 6) | (((uint64_t)(in_cam[76] & 0x7FF)) << 5) |
                      (((uint64_t)(in_cam[77] & 0x7FF)) << 16) | (((uint64_t)(in_cam[78] & 0x7FF)) << 27) |
                      (((uint64_t)(in_cam[79] & 0x7FF)) << 38) | (((uint64_t)(in_cam[80] & 0x7FF)) << 49) |
                      (((uint64_t)(in_cam[81] & 0x7FF)) << 60);
    tmp_cam_ptr[12] = (((uint64_t)(in_cam[81] & 0x7FF)) >> 4) | (((uint64_t)(in_cam[82] & 0x7FF)) << 7) |
                      (((uint64_t)(in_cam[83] & 0x7FF)) << 18) | (((uint64_t)(in_cam[84] & 0x7FF)) << 29) |
                      (((uint64_t)(in_cam[85] & 0x7FF)) << 40) | (((uint64_t)(in_cam[86] & 0x7FF)) << 51) |
                      (((uint64_t)(in_cam[87] & 0x7FF)) << 62);
    tmp_cam_ptr[13] = (((uint64_t)(in_cam[87] & 0x7FF)) >> 2) | (((uint64_t)(in_cam[88] & 0x7FF)) << 9) |
                      (((uint64_t)(in_cam[89] & 0x7FF)) << 20) | (((uint64_t)(in_cam[90] & 0x7FF)) << 31) |
                      (((uint64_t)(in_cam[91] & 0x7FF)) << 42) | (((uint64_t)(in_cam[92] & 0x7FF)) << 53);
    tmp_cam_ptr[14] = (((uint64_t)(in_cam[93] & 0x7FF)) << 0) | (((uint64_t)(in_cam[94] & 0x7FF)) << 11) |
                      (((uint64_t)(in_cam[95] & 0x7FF)) << 22) | (((uint64_t)(in_cam[96] & 0x7FF)) << 33) |
                      (((uint64_t)(in_cam[97] & 0x7FF)) << 44) | (((uint64_t)(in_cam[98] & 0x7FF)) << 55);
    tmp_cam_ptr[15] = (((uint64_t)(in_cam[98] & 0x7FF)) >> 9) | (((uint64_t)(in_cam[99] & 0x7FF)) << 2) |
                      (((uint64_t)(in_cam[100] & 0x7FF)) << 13) | (((uint64_t)(in_cam[101] & 0x7FF)) << 24) |
                      (((uint64_t)(in_cam[102] & 0x7FF)) << 35) | (((uint64_t)(in_cam[103] & 0x7FF)) << 46) |
                      (((uint64_t)(in_cam[104] & 0x7FF)) << 57);
    tmp_cam_ptr[16] = (((uint64_t)(in_cam[104] & 0x7FF)) >> 7) | (((uint64_t)(in_cam[105] & 0x7FF)) << 4) |
                      (((uint64_t)(in_cam[106] & 0x7FF)) << 15) | (((uint64_t)(in_cam[107] & 0x7FF)) << 26) |
                      (((uint64_t)(in_cam[108] & 0x7FF)) << 37) | (((uint64_t)(in_cam[109] & 0x7FF)) << 48) |
                      (((uint64_t)(in_cam[110] & 0x7FF)) << 59);
    tmp_cam_ptr[17] = (((uint64_t)(in_cam[110] & 0x7FF)) >> 5) | (((uint64_t)(in_cam[111] & 0x7FF)) << 6) |
                      (((uint64_t)(in_cam[112] & 0x7FF)) << 17) | (((uint64_t)(in_cam[113] & 0x7FF)) << 28) |
                      (((uint64_t)(in_cam[114] & 0x7FF)) << 39) | (((uint64_t)(in_cam[115] & 0x7FF)) << 50) |
                      (((uint64_t)(in_cam[116] & 0x7FF)) << 61);
    tmp_cam_ptr[18] = (((uint64_t)(in_cam[116] & 0x7FF)) >> 3) | (((uint64_t)(in_cam[117] & 0x7FF)) << 8) |
                      (((uint64_t)(in_cam[118] & 0x7FF)) << 19) | (((uint64_t)(in_cam[119] & 0x7FF)) << 30) |
                      (((uint64_t)(in_cam[120] & 0x7FF)) << 41) | (((uint64_t)(in_cam[121] & 0x7FF)) << 52) |
                      (((uint64_t)(in_cam[122] & 0x7FF)) << 63);
    tmp_cam_ptr[19] = (((uint64_t)(in_cam[122] & 0x7FF)) >> 1) | (((uint64_t)(in_cam[123] & 0x7FF)) << 10) |
                      (((uint64_t)(in_cam[124] & 0x7FF)) << 21) | (((uint64_t)(in_cam[125] & 0x7FF)) << 32) |
                      (((uint64_t)(in_cam[126] & 0x7FF)) << 43) | (((uint64_t)(in_cam[127] & 0x7FF)) << 54);
    tmp_cam_ptr[20] = (((uint64_t)(in_cam[127] & 0x7FF)) >> 10) | (((uint64_t)(in_cam[128] & 0xFFF)) << 1) |
                      (((uint64_t)(in_cam[129] & 0xFFF)) << 13) | (((uint64_t)(in_cam[130] & 0xFFF)) << 25) |
                      (((uint64_t)(in_cam[131] & 0xFFF)) << 37) | (((uint64_t)(in_cam[132] & 0xFFF)) << 49) |
                      (((uint64_t)(in_cam[133] & 0xFFF)) << 61);

    uint32_t out = 0U, in = 0U;

    for (out = 21, in = 133; out < 42; out += 3, in += 16) {
        tmp_cam_ptr[out] = (((uint64_t)(in_cam[in + 0] & 0xFFF)) >> 3) | (((uint64_t)(in_cam[in + 1] & 0xFFF)) << 9) |
                           (((uint64_t)(in_cam[in + 2] & 0xFFF)) << 21) | (((uint64_t)(in_cam[in + 3] & 0xFFF)) << 33) |
                           (((uint64_t)(in_cam[in + 4] & 0xFFF)) << 45) | (((uint64_t)(in_cam[in + 5] & 0xFFF)) << 57);
        if (out == 27) {

            // Change pointer here to point to the second part
            // of the LL Mapping CAM
            tmp_cam_ptr = (uint64_t*)(out_cam_p2 - 224);
        }
        tmp_cam_ptr[out + 1] =
                (((uint64_t)(in_cam[in + 5] & 0xFFF)) >> 7) | (((uint64_t)(in_cam[in + 6] & 0xFFF)) << 5) |
                (((uint64_t)(in_cam[in + 7] & 0xFFF)) << 17) | (((uint64_t)(in_cam[in + 8] & 0xFFF)) << 29) |
                (((uint64_t)(in_cam[in + 9] & 0xFFF)) << 41) | (((uint64_t)(in_cam[in + 10] & 0xFFF)) << 53);
        tmp_cam_ptr[out + 2] =
                (((uint64_t)(in_cam[in + 10] & 0xFFF)) >> 11) | (((uint64_t)(in_cam[in + 11] & 0xFFF)) << 1) |
                (((uint64_t)(in_cam[in + 12] & 0xFFF)) << 13) | (((uint64_t)(in_cam[in + 13] & 0xFFF)) << 25) |
                (((uint64_t)(in_cam[in + 14] & 0xFFF)) << 37) | (((uint64_t)(in_cam[in + 15] & 0xFFF)) << 49) |
                (((uint64_t)(in_cam[in + 16] & 0xFFF)) << 61);
    }
    tmp_cam_ptr[42] = (((uint64_t)(in_cam[245] & 0xFFF)) >> 3) | (((uint64_t)(in_cam[246] & 0xFFF)) << 9) |
                      (((uint64_t)(in_cam[247] & 0xFFF)) << 21) | (((uint64_t)(in_cam[248] & 0xFFF)) << 33) |
                      (((uint64_t)(in_cam[249] & 0xFFF)) << 45) | (((uint64_t)(in_cam[250] & 0xFFF)) << 57);
    tmp_cam_ptr[43] = (((uint64_t)(in_cam[250] & 0xFFF)) >> 7) | (((uint64_t)(in_cam[251] & 0xFFF)) << 5) |
                      (((uint64_t)(in_cam[252] & 0xFFF)) << 17) | (((uint64_t)(in_cam[253] & 0xFFF)) << 29) |
                      (((uint64_t)(in_cam[254] & 0xFFF)) << 41) | (((uint64_t)(in_cam[255] & 0xFFF)) << 53);
    tmp_cam_ptr[44] = (((uint64_t)(in_cam[255] & 0xFFF)) >> 11) | (((uint64_t)(in_cam[256] & 0x1FFF)) << 1) |
                      (((uint64_t)(in_cam[257] & 0x1FFF)) << 14) | (((uint64_t)(in_cam[258] & 0x1FFF)) << 27) |
                      (((uint64_t)(in_cam[259] & 0x1FFF)) << 40) | (((uint64_t)(in_cam[260] & 0x1FFF)) << 53);
    tmp_cam_ptr[45] = (((uint64_t)(in_cam[260] & 0x1FFF)) >> 11) | (((uint64_t)(in_cam[261] & 0x1FFF)) << 2) |
                      (((uint64_t)(in_cam[262] & 0x1FFF)) << 15) | (((uint64_t)(in_cam[263] & 0x1FFF)) << 28) |
                      (((uint64_t)(in_cam[264] & 0x1FFF)) << 41);
}

HW_PATH_IAA_AECS_API(void, decompress_set_huffman_only_huffman_table,
                     (hw_iaa_aecs_decompress* const aecs_ptr, hw_iaa_d_huffman_only_table* const huffman_table_ptr)) {

    call_c_set_zeros_uint8_t((uint8_t*)aecs_ptr, sizeof(hw_iaa_aecs_analytic));

    aecs_ptr->lit_len_first_len_code[0] = 0x07FFFFFFU;
    aecs_ptr->lit_len_first_len_code[1] = 0x07FFFFFFU;
    aecs_ptr->lit_len_first_len_code[2] = 0x007FFFFFU;
    aecs_ptr->lit_len_first_len_code[3] = 0x07FFFFFFU;
    aecs_ptr->lit_len_first_len_code[4] = 0x7FFFFFFFU;

    pack_table(aecs_ptr->lit_len_num_codes, huffman_table_ptr->number_of_codes);
    pack_table(aecs_ptr->lit_len_first_code, huffman_table_ptr->first_codes);

    /* Huffman table would store either mapping table or mapping CAM
       (based on the IAACAP bit 0, or, in other words, based on the AECS Format supported on the accelerator).
       AECS Format-1: requires packing mapping table and first table indices;
       AECS Format-2: requires mapping CAM packed (in irregular way, not plain copy);
       All other information is used for both AECS Format-1, 2.
    */
    if (huffman_table_ptr->format_stored == ht_with_mapping_table) {
        pack_table(aecs_ptr->lit_len_first_tbl_idx, huffman_table_ptr->first_table_indexes);
        call_c_copy_uint8_t((uint8_t*)huffman_table_ptr->index_to_char, (uint8_t*)aecs_ptr->lit_len_sym, 256U);
    } else {
        pack_cam(aecs_ptr->ll_mapping_cam_1, aecs_ptr->ll_mapping_cam_2, huffman_table_ptr->lit_cam);
    }

    hw_iaa_aecs_decompress_state_set_aecs_format(aecs_ptr, (huffman_table_ptr->format_stored == ht_with_mapping_cam));
}

/**
 * @brief Routine to transform AECS compression state to decompression for Huffman Only.
 *
 * @details Here Huffman Codes are stored in hw_iaa_histogram->ll_sym structure.
 * Goal is to build either Mapping Table and First Table Indices array or Mapping CAM.
 * Also construct Number of Codes and First Codes arrays (used in both Formats).
 *
 * Mapping table (corresponds to AECS Format-1) is such that:
 * we store all the symbols with length 1 first (sorted), then all the symbols with length 2 (sorted), etc.
 * For a given code length i, number of such codes is num_codes[i],
 * region with these codes in the mapping table starts with first_tbl_idx[i] offset,
 * and additionally we store first_code[i] for the code with length i that is first once sorted.
 * Table index for certain input code of length i could be computed then as:
 * first_tbl_idx[i] + input code - first_code[i].
 *
 * Mapping CAM (corresponds to AECS Format-2) is such that, for each entry
 * the index is the input symbol and the value is the pair of input code length and (input code - first code),
 * stored in first 4 bits and next 4 bits respectively;
 * Therefore CAM size is exactly 265 (number of len codes without those requiring extra bits).
 * Working with Mapping CAM requires num_codes and first_codes arrays,
 * but doesn't require first_tbl_idx (as calculating offset is not needed).
 *
 * In AECS for Compress Huffman Codes are stored such that in hw_iaa_histogram->ll_sym entry is a code,
 * its index is a symbol, and bits 18:15 is a length.
*/
HW_PATH_IAA_AECS_API(uint32_t, decompress_set_huffman_only_huffman_table_from_histogram,
                     (hw_iaa_aecs_decompress* const aecs_ptr, const hw_iaa_histogram* const histogram_ptr,
                      bool is_aecs_format2_expected)) {
    uint32_t idx  = 0U;
    uint32_t code = 0U, len = 0U;

    // common for Mapping Table and Mapping CAM representations
    uint16_t num_codes[16];
    uint16_t first_code[16];
    uint16_t next_code[16];

    // AECS Format-1 specifics
    uint16_t first_tbl_idx[16];
    uint8_t* lit_len_sym = NULL;

    // AECS Format-2 specifics
    uint16_t lit_cam[265];
    uint16_t cam_offset[16];

    const uint32_t* const ll_huffman_table = histogram_ptr->ll_sym;
    aecs_ptr->decompress_state             = DEF_STATE_LL_TOKEN;

    // Initialization
    num_codes[0] = 0U;
    for (idx = 1U; idx <= 15U; idx++) {
        num_codes[idx] = first_code[idx] = next_code[idx] = 0U;
    }

    if (is_aecs_format2_expected) {
        for (idx = 1U; idx <= 15U; idx++) {
            cam_offset[idx] = 0U;
        }

        call_c_set_zeros_uint8_t((uint8_t*)lit_cam, sizeof(lit_cam));
    }

    // Iterate through all 256 codes
    // and fill in common parts as well as Mapping CAM
    for (idx = 0U; idx < 256U; idx++) {
        code = ll_huffman_table[idx];
        len  = code >> 15U;
        if (0U == len) { continue; }
        if (15U < len) { return 2U; }
        code &= 0x7FFFU;
        if (0U == num_codes[len]) {
            // First Time a certain length code is met
            num_codes[len]  = 1U;
            first_code[len] = code;
            next_code[len]  = code + 1U;
        } else {
            num_codes[len]++;
            if (code != next_code[len]) { return 1U; }
            next_code[len]++;
        }

        if (is_aecs_format2_expected) {
            // cam_offset to store entries of certain length in a sorted way
            lit_cam[idx] = len | ((cam_offset[len]++) << 4);
        }
    }

    // Pack Mapping CAM to AECS for Decompress
    if (is_aecs_format2_expected) {
        pack_cam(aecs_ptr->ll_mapping_cam_1, aecs_ptr->ll_mapping_cam_2, lit_cam);
    } else { // Compute and pack First Table Indices and Mapping Table
        first_tbl_idx[0] = 0U;
        for (idx = 0U; idx < 15U; idx++) {
            first_tbl_idx[idx + 1] = first_tbl_idx[idx] + num_codes[idx];
        }
        pack_table(aecs_ptr->lit_len_first_tbl_idx, first_tbl_idx + 1U);

        lit_len_sym = aecs_ptr->lit_len_sym;
        for (idx = 0U; idx < 256U; idx++) {
            len = ll_huffman_table[idx] >> 15U;
            if (len != 0U) { lit_len_sym[first_tbl_idx[len]++] = idx; }
        }
    }

    // Pack common parts
    pack_table(aecs_ptr->lit_len_num_codes, num_codes + 1U);
    pack_table(aecs_ptr->lit_len_first_code, first_code + 1U);
    aecs_ptr->lit_len_first_len_code[0] = 0x07FFFFFFU;
    aecs_ptr->lit_len_first_len_code[1] = 0x07FFFFFFU;
    aecs_ptr->lit_len_first_len_code[2] = 0x007FFFFFU;
    aecs_ptr->lit_len_first_len_code[3] = 0x07FFFFFFU;
    aecs_ptr->lit_len_first_len_code[4] = 0x7FFFFFFFU;

    return 0;
}

HW_PATH_IAA_AECS_API(void, decompress_set_dictionary,
                     (hw_iaa_aecs_decompress* const aecs_ptr, const uint8_t* const raw_dictionary_ptr,
                      const size_t raw_dictionary_size, const size_t decompress_dictionary_size,
                      const uint32_t decompress_raw_dictionary_offset)) {
    // Zero the bytes in the history buffer before the raw dictionary data starts
    call_c_set_zeros_uint8_t(aecs_ptr->history_buffer, decompress_raw_dictionary_offset);

    // Copy the raw dictionary data into the history buffer
    for (uint32_t i = 0U; i < raw_dictionary_size; i++) {
        aecs_ptr->history_buffer[decompress_raw_dictionary_offset + i] = raw_dictionary_ptr[i];
    }

    aecs_ptr->history_buffer_params.history_buffer_write_offset = (uint16_t)decompress_dictionary_size;

    if (decompress_dictionary_size >= 4096U) { aecs_ptr->history_buffer_params.is_history_buffer_overflowed = 1; }
}

HW_PATH_IAA_AECS_API(uint32_t, decompress_set_input_accumulator,
                     (hw_iaa_aecs_decompress* const aecs_ptr, const uint8_t* const source_ptr,
                      const uint32_t source_size, const uint8_t ignore_start_bits, const uint8_t ignore_end_bits)) {
    uint32_t i = 0U;
    for (i = 0U; i < OWN_INFLATE_INPUT_ACCUMULATOR_DQ_COUNT - 1U; i++) {
        if (0U == aecs_ptr->input_accum_size[i]) { break; }
    }

    HW_IMMEDIATELY_RET((0U != aecs_ptr->input_accum_size[i]), OWN_STATUS_ERROR)

    if (1U < source_size) {
        aecs_ptr->input_accum[i]      = (*source_ptr) >> (ignore_start_bits & OWN_MAX_BIT_IDX);
        aecs_ptr->input_accum_size[i] = 8U - (ignore_start_bits & 7U);
    } else {
        HW_IMMEDIATELY_RET((1 > source_size), OWN_STATUS_ERROR)

        aecs_ptr->input_accum[i]      = (*source_ptr) >> (ignore_start_bits & OWN_MAX_BIT_IDX);
        aecs_ptr->input_accum_size[i] = OWN_MAX_BIT_IDX & (0U - (int32_t)ignore_start_bits - (int32_t)ignore_end_bits);
        aecs_ptr->input_accum[i] &= (1U << aecs_ptr->input_accum_size[i]) - 1U;
    }

    return OWN_STATUS_OK;
}

HW_PATH_IAA_AECS_API(void, decompress_clean_input_accumulator, (hw_iaa_aecs_decompress* const aecs_ptr)) {
    // If input_accum_size is set to 0, no need to reset input_accum
    call_c_set_zeros_uint8_t(aecs_ptr->input_accum_size, sizeof(aecs_ptr->input_accum_size));
}

HW_PATH_IAA_AECS_API(void, decompress_clean_aecs, (hw_iaa_aecs_decompress* const aecs_ptr)) {
    // Clean necessary fields in the decompress AECS to avoid using corrupted data from previous unrelated job
    aecs_ptr->output_acc_bits_valid = 0U;

    aecs_ptr->idx_bit_offset = 0U;

    hw_iaa_aecs_decompress_clean_input_accumulator(aecs_ptr);

    aecs_ptr->drop_initial_bits = 0U;

    aecs_ptr->reserved4 = 0U;

    aecs_ptr->reserved5[0] = 0U;
    aecs_ptr->reserved5[1] = 0U;

    aecs_ptr->history_buffer_params.history_buffer_write_offset  = 0U;
    aecs_ptr->history_buffer_params.is_history_buffer_overflowed = 0U;
}
