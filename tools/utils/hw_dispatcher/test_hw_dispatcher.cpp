/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * HW dispatcher APIs for tests
 */

#include "test_hw_dispatcher.hpp"

namespace qpl::test {
hw_dispatcher::hw_dispatcher() noexcept : hw_init_status_(hw_dispatcher::initialize_hw()) {
    hw_support_ = hw_init_status_ == QPL_TEST_HW_ACCELERATOR_STATUS_OK;
}

auto hw_dispatcher::initialize_hw() noexcept -> qpl_test_hw_accelerator_status {
#if defined(__linux__)
    accfg_ctx* ctx_ptr = nullptr;

#ifdef DYNAMIC_LOADING_LIBACCEL_CONFIG
    const qpl_test_hw_accelerator_status status = qpl_test_hw_initialize_accelerator_driver(&hw_driver_);
    if (status != QPL_TEST_HW_ACCELERATOR_STATUS_OK) return status;
#endif

    const int32_t context_creation_status = qpl_test_accfg_new(&ctx_ptr);
    if (0U != context_creation_status) return QPL_TEST_HW_ACCELERATOR_LIBACCEL_ERROR;

    // Retrieve first device in the system given the passed in context
    auto* dev_tmp_ptr = qpl_test_accfg_device_get_first(ctx_ptr);
    auto  device_it   = devices_.begin();

    while (nullptr != dev_tmp_ptr) {
        if (QPL_TEST_HW_ACCELERATOR_STATUS_OK == device_it->initialize_new_device(dev_tmp_ptr)) { device_it++; }

        // Retrieve the "next" device in the system based on given device
        dev_tmp_ptr = qpl_test_accfg_device_get_next(dev_tmp_ptr);
    }

    device_count_ = std::distance(devices_.begin(), device_it);

    if (device_count_ <= 0) {
        return QPL_TEST_HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE; // No devices -> No WQ
    }

    hw_context_.set_driver_context_ptr(ctx_ptr);

    return QPL_TEST_HW_ACCELERATOR_STATUS_OK;
#else
    // Windows is not supported
    return QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND;
#endif
}

hw_dispatcher::~hw_dispatcher() noexcept {
#if defined(__linux__)
    // Variables
    auto* context_ptr = hw_context_.get_driver_context_ptr();

    if (context_ptr != nullptr) { qpl_test_accfg_unref(context_ptr); }

#ifdef DYNAMIC_LOADING_LIBACCEL_CONFIG
    qpl_test_hw_finalize_accelerator_driver(&hw_driver_);
#endif

    // Zeroing values
    hw_context_.set_driver_context_ptr(nullptr);
#endif
}

// starting from C++11 standard,
// it is guaranteed that the following would be thread-safe
// and created only once
// (case: static variables with block scope)
auto hw_dispatcher::get_instance() noexcept -> hw_dispatcher& {
    static hw_dispatcher instance {};
    return instance;
}

auto hw_dispatcher::get_hw_init_status() const noexcept -> qpl_test_hw_accelerator_status {
    return hw_init_status_;
}

auto hw_dispatcher::is_hw_support() const noexcept -> bool {
    return hw_support_;
}

#if defined(__linux__)

auto hw_dispatcher::begin() const noexcept -> device_container_t::const_iterator {
    return devices_.cbegin();
}

auto hw_dispatcher::end() const noexcept -> device_container_t::const_iterator {
    return devices_.cbegin() + device_count_;
}

auto hw_dispatcher::device(size_t idx) const noexcept -> const hw_device& {
    return devices_[idx % device_count_];
}

auto hw_dispatcher::device_count() const noexcept -> size_t {
    return device_count_;
}

void hw_dispatcher::hw_context::set_driver_context_ptr(accfg_ctx* driver_context_ptr) noexcept {
    driver_context_ptr_ = driver_context_ptr;
}

[[nodiscard]] auto hw_dispatcher::hw_context::get_driver_context_ptr() noexcept -> accfg_ctx* {
    return driver_context_ptr_;
}

#endif
} // namespace qpl::test
