/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_UTIL_HW_TIMING_UTIL_HPP_
#define QPL_SOURCES_MIDDLE_LAYER_UTIL_HW_TIMING_UTIL_HPP_

#ifdef QPL_LOG_IAA_TIME

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "hw_definitions.h"
#include "hw_iaa_flags.h"

namespace qpl::ml::dispatcher {

// The offset of opcode in the field that packs (Opcode 31:24, opflags 23:0)
#define QPL_OPCODE_OFFSET_IN_OPCODE_OPFLAGS 24U

struct iaa_operation_timestamp {
    uint8_t                                                     operation;
    uint32_t                                                    src_size;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
};

// Global vector of structures that hold the timestamps and meta data
// for operations executed on Intel® In-Memory Analytics Accelerator (Intel® IAA).
// Global vector is defined in sources/middle-layer/dispatcher/hw_dispatcher.cpp.
extern std::vector<iaa_operation_timestamp> iaa_timestamps;

/**
 * @brief Records the meta data for operation executed on Intel IAA
 * by adding a new entry to the vector that stores operation timestamps, and
 * setting the fields.
 *
 * @note The application must be single-threaded to ensure accurate
 * correspondence between recorded values.
*/
inline void record_meta_data(void* desc_ptr) {
    // The locations of opcode and source 1 size are the same in
    // descriptor structures of different operation types,
    // so just cast into hw_compress_descriptor
    hw_compress_descriptor* const this_ptr         = (hw_compress_descriptor*)desc_ptr;
    uint32_t                      op_code_op_flags = this_ptr->op_code_op_flags;
    uint8_t                       op = static_cast<uint8_t>(op_code_op_flags >> QPL_OPCODE_OFFSET_IN_OPCODE_OPFLAGS);
    uint32_t                      source_size = this_ptr->source_size;

    iaa_operation_timestamp iaa_op_timestamp;
    iaa_op_timestamp.operation = op;
    iaa_op_timestamp.src_size  = source_size;
    iaa_timestamps.push_back(iaa_op_timestamp);
}

/**
 * @brief Records the current time as the start time for operation executed
 * on Intel IAA by setting the start time field in the most recent entry of the vector
 * that stores operation timestamps.
 *
 * @note The application must be single-threaded to ensure accurate
 * correspondence between recorded start times and end times.
*/
inline void record_iaa_start_time() {
    if (!iaa_timestamps.empty()) iaa_timestamps.back().start_time = std::chrono::high_resolution_clock::now();
}

/**
 * @brief Records the current time as the end time for operation executed
 * on Intel IAA by setting the end time field in the most recent entry of the vector
 * that stores operation timestamps.
 *
 * @note The application must be single-threaded to ensure accurate
 * correspondence between recorded start times and end times.
*/
inline void record_iaa_end_time() {
    if (!iaa_timestamps.empty()) iaa_timestamps.back().end_time = std::chrono::high_resolution_clock::now();
}

/**
 * @brief Skips the time measurement of Intel IAA by storing an invalid
 * value as the end time. For example, since execution in async path
 * is non-blocking, accurate time measurement is not possible. Therefore, timing
 * should be skipped.
*/
inline void record_invalid_end_time_to_skip_iaa_timing() {
    if (!iaa_timestamps.empty())
        iaa_timestamps.back().end_time = std::chrono::time_point<std::chrono::high_resolution_clock>::min();
}

inline std::string get_name_from_opcode(uint8_t opcode) {
    switch (opcode) {
        case QPL_OPCODE_DECOMPRESS: return "decompress";
        case QPL_OPCODE_COMPRESS: return "compress";
        case QPL_OPCODE_CRC64: return "crc64";
        case QPL_OPCODE_SCAN: return "scan";
        case QPL_OPCODE_EXTRACT: return "extract";
        case QPL_OPCODE_SELECT: return "select";
        case QPL_OPCODE_EXPAND: return "expand";
        default: return "unsupported operation";
    }
}

static std::ostream& operator<<(std::ostream& os, const iaa_operation_timestamp& timestamp) {
    auto   elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp.end_time - timestamp.start_time);
    double elapsedTime    = elapsed.count();
    std::string operation = get_name_from_opcode(timestamp.operation);

    os << "Time taken to complete HW descriptor: " << elapsedTime << " ns, operation: " << operation
       << ", source size: " << timestamp.src_size;

    return os;
}

/**
 * @brief Calculates the elapsed time for execution on Intel IAA by finding the
 * differences between the recorded start times and end times.
 *
 * This function is called in the destructor of the singleton hw_dispatcher
 * object to output the timing values before the application terminates.
 *
 * @note The application must be single-threaded to ensure accurate
 * correspondence between recorded start times and end times
*/
inline void calculate_iaa_elapsed_time() {
    for (std::size_t i = 0; i < iaa_timestamps.size(); ++i) {
        if (iaa_timestamps[i].end_time != std::chrono::time_point<std::chrono::high_resolution_clock>::min()) {
            std::cout << "#" << i << ": " << iaa_timestamps[i] << "\n";
        }
    }
}

} // namespace qpl::ml::dispatcher

#endif //QPL_LOG_IAA_TIME
#endif //QPL_SOURCES_MIDDLE_LAYER_UTIL_HW_TIMING_UTIL_HPP_
