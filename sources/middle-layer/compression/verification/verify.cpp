/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "verify.hpp"

#include "compression/huffman_only/huffman_only.hpp"
#include "verification_units.hpp"

namespace qpl::ml::compression {

// ------ SOFTWARE PATH ------ //
template <>
auto perform_verification<execution_path_t::software, verification_mode_t::verify_deflate_no_headers>(
        verify_state<execution_path_t::software>& state) -> verification_result_t {
    bool                  break_loop = false;
    verification_result_t verification_result;

    while (state.get_input_size() > 0 && !break_loop) {
        verification_result += verify_deflate_stream_body(state);

        switch (verification_result.status) {
            // Output buffer filled: recycle it while preserving the 32 KB history window so a
            // match spanning the recycle boundary can still resolve its look-back.
            case parser_status_t::end_of_mini_block: state.slide_output_buffer(); break;
            case parser_status_t::end_of_block:
            case parser_status_t::final_end_of_block:
                state.slide_output_buffer();
                break_loop = true;
                break;
            case parser_status_t::error: return verification_result;
            default: break_loop = true; break;
        }

        state.get_state()->crc = verification_result.crc_value;
    }

    if (verification_result.status == parser_status_t::final_end_of_block ||
        verification_result.status == parser_status_t::end_of_block ||
        verification_result.status == parser_status_t::end_of_mini_block) {
        if (state.get_state()->crc != state.get_required_crc()) { verification_result.status = parser_status_t::error; }
    }

    return verification_result;
}

template <>
auto perform_verification<execution_path_t::software, verification_mode_t::verify_deflate_default>(
        verify_state<execution_path_t::software>& state) -> verification_result_t {
    verification_result_t verification_result {};
    bool                  break_loop = false;

    while (state.get_input_size() > 0 && !break_loop) {
        if (state.get_parser_position() == parser_position_t::verify_header) {
            verification_result += verify_deflate_header(state);

            if (verification_result.status != parser_status_t::ok) { return verification_result; }

            state.set_parser_position(parser_position_t::verify_body);
        } else {
            verification_result += verify_deflate_stream_body(state);

            switch (verification_result.status) {
                // Recycle the output buffer while preserving the 32 KB history window: deflate's
                // LZ77 back-references persist across block boundaries and buffer recycles, so a
                // match may look back into output produced before this point.
                case parser_status_t::end_of_block:
                    state.slide_output_buffer();
                    state.set_parser_position(parser_position_t::verify_header);
                    break;
                case parser_status_t::end_of_mini_block: state.slide_output_buffer(); break;
                case parser_status_t::final_end_of_block:
                    state.slide_output_buffer();
                    state.set_parser_position(parser_position_t::verify_header);
                    break_loop = true;
                    break;
                case parser_status_t::error: return verification_result;
                default: break_loop = true; break;
            }

            state.get_state()->crc = verification_result.crc_value;
        }
    }

    if (verification_result.status == parser_status_t::final_end_of_block ||
        verification_result.status == parser_status_t::end_of_block ||
        verification_result.status == parser_status_t::end_of_mini_block) {
        if (state.get_state()->crc != state.get_required_crc()) { verification_result.status = parser_status_t::error; }
    }

    return verification_result;
}

// ------ HARDWARE PATH ------ //
template <>
auto perform_verification<execution_path_t::hardware, verification_mode_t::verify_deflate_default>(
        verify_state<execution_path_t::hardware>& UNREFERENCED_PARAMETER(state)) -> verification_result_t {
    // Implementation here ...
    verification_result_t verification_result;
    return verification_result;
}

template <>
auto perform_verification<execution_path_t::hardware, verification_mode_t::verify_deflate_no_headers>(
        verify_state<execution_path_t::hardware>& UNREFERENCED_PARAMETER(state)) -> verification_result_t {
    // Implementation here ...
    verification_result_t verification_result;
    return verification_result;
}
} // namespace qpl::ml::compression
