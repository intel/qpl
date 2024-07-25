/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <ops/results.hpp>
#include <types.hpp>

namespace bench::ops {
template <typename Type>
struct traits {};

template <typename DerivedT>
class operation_base_t {
public:
    operation_base_t() noexcept {}
    ~operation_base_t() noexcept {}

    // Init library and set data for the measurement loop
    // Init has 3 parts:
    // - Initialization of operation buffers (not included in full_time)
    // - Initialization of operation context (included in full_time)
    // - Initialization of operation arguments (included in full_time)
    template <typename ParamsT>
    void init(const ParamsT& params, std::int32_t numa_id = -1, bool cache_control = true, bool full_time = false) {
        cache_control_ = cache_control;
        numa_id_       = numa_id;
        full_time_     = full_time;

        derived().deinit_lib_impl();         // Check initialization to be cleared
        derived().init_buffers_impl(params); // Init operation specific buffers
        if (full_time_) return;

        derived().init_lib_impl();        // Init library specific context
        derived().init_lib_params_impl(); // Init operation specific arguments
    }

    // Synchronous operation execution
    void sync_execute() {
        if (full_time_) {
            derived().init_lib_impl();
            derived().init_lib_params_impl();
        }

        retired_ = true;
        derived().sync_execute_impl();

        if (full_time_) derived().deinit_lib_impl();
    }

    // Asynchronous operation execution
    void async_submit() {
        if (full_time_) {
            derived().init_lib_impl();
            derived().init_lib_params_impl();
        }

        retired_ = false;
        derived().async_submit_impl();
    }

    // Blocking wait for asynchronous operation
    task_status_e async_wait() {
        if (retired_) return task_status_e::retired;

        auto result = derived().async_wait_impl();
        if (result == task_status_e::completed) {
            if (full_time_) derived().deinit_lib_impl();
            retired_ = true;
        }
        return result;
    }

    // Non-blocking wait for asynchronous operation
    [[nodiscard]] task_status_e async_poll() {
        if (retired_) return task_status_e::retired;

        auto result = derived().async_poll_impl();
        if (result == task_status_e::completed) {
            if (full_time_) derived().deinit_lib_impl();
            retired_ = true;
        }
        return result;
    }

    // Lightweight reset of the operation
    void light_reset() noexcept {
        if (full_time_) return;

        derived().light_reset_impl();
    }

    // Set data location
    void mem_control(mem_loc_e op, mem_loc_mask_e mask) const noexcept { derived().mem_control_impl(op, mask); }

    // Get result vectors
    auto get_result() noexcept -> typename traits<operation_base_t>::result_t& { return derived().get_result_impl(); }

    // Getters for amount of processed bytes for throughput stats
    size_t get_bytes_read() { return bytes_read_; }
    size_t get_bytes_written() { return bytes_written_; }

protected:
    const DerivedT& derived() const { return *static_cast<const DerivedT*>(this); }
    DerivedT&       derived() { return *static_cast<DerivedT*>(this); }

    bool         cache_control_ {true};
    bool         full_time_ {false};
    bool         retired_ {true};
    std::int32_t numa_id_ {-1};
    std::size_t  bytes_read_ {0};
    std::size_t  bytes_written_ {0};
};
} // namespace bench::ops
