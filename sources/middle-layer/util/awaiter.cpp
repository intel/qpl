/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "awaiter.hpp"

#ifdef QPL_LOG_IAA_TIME
#include "hw_timing_util.hpp"
#endif

#if defined(__linux__)

#include <x86intrin.h>

#else
#include <emmintrin.h>
#include <intrin.h>
#endif

/**
 * Unused variables which might be used later - warning removal
 */
#define MAYBE_UNUSED(x) ((void)(x))

namespace qpl::ml {

#ifdef QPL_EFFICIENT_WAIT
static inline uint64_t current_time() {
    return __rdtsc();
}

static inline void monitor_address(volatile void* address) {
    asm volatile(".byte 0xf3, 0x48, 0x0f, 0xae, 0xf0" : : "a"(address));
}

static inline int wait_until(uint64_t timeout, uint32_t state) {
    uint8_t r            = 0u;
    auto    timeout_low  = static_cast<uint32_t>(timeout);
    auto    timeout_high = static_cast<uint32_t>(timeout >> 32);

    asm volatile(
            ".byte 0xf2, 0x48, 0x0f, 0xae, 0xf1\t\n"
            "setc %0\t\n"
            : "=r"(r)
            : "c"(state), "a"(timeout_low), "d"(timeout_high));

    return r;
}
#endif

awaiter::awaiter(volatile void* address, uint8_t initial_value, uint32_t period) noexcept
    : address_ptr_(reinterpret_cast<volatile uint8_t*>(address)), period_(period), initial_value_(initial_value) {
    // Empty constructor
}

awaiter::~awaiter() noexcept {
#ifdef QPL_EFFICIENT_WAIT
    while (initial_value_ == *address_ptr_) {
        monitor_address(address_ptr_);

        auto start = current_time();
        wait_until(start + period_, idle_state_);
    }
#else
    MAYBE_UNUSED(period_);
    MAYBE_UNUSED(idle_state_);
    while (initial_value_ == *address_ptr_) {
        _mm_pause();
    }
#endif

#ifdef QPL_LOG_IAA_TIME
    // Record the end time right after descriptor is completed
    dispatcher::record_iaa_end_time();
#endif
}

void awaiter::wait_for(volatile void* address, uint8_t initial_value) noexcept {
    const awaiter wait_for(address, initial_value);
}
} // namespace qpl::ml
