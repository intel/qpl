/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <memory>
#include <array>

#ifndef QPL_EXAMPLES_HIGH_LEVEL_API_UTILS_STACK_ALLOCATOR_HPP_
#define QPL_EXAMPLES_HIGH_LEVEL_API_UTILS_STACK_ALLOCATOR_HPP_

/**
 * @brief Implements a simple memory buffer that works as stack.
 * @tparam T Type of elements contained by buffer
 */
template <class T>
class memory_blocks_stack {
    /**
     * @brief Defines maximal allocation buffer
     */
    static constexpr uint32_t STACK_SIZE = (1u * 1024u * 1024u) / sizeof(T); /**< 1 MiB */

public:
    /**
     * @brief Copy constructor is not available for singleton object
     */
    memory_blocks_stack(const memory_blocks_stack &) = delete;

    /**
     * @brief Assignment operator is not available for singleton object
     */
    memory_blocks_stack &operator=(const memory_blocks_stack) = delete;

    /**
     * @brief Removes last pushed block
     */
    void pop() noexcept {
        stack_pointer_ -= sizeof(size_t);
        size_t size = *((size_t *) stack_pointer_);
        stack_pointer_ -= (size * sizeof(T));
    }

    /**
     * @brief Adds a new memory block on the top of stack
     * @param[in] size count of type `T` elements pushed
     */
    void push(size_t size) noexcept {
        stack_pointer_ += size * sizeof(T);
        *((size_t *) stack_pointer_) = size;
        stack_pointer_ += sizeof(size_t);
    }

    /**
     * @brief Returns current top of the stack
     * @return Address pointed by stack pointer
     */
    [[nodiscard]] auto stack_pointer() noexcept -> T * {
        return stack_pointer_;
    }

    /**
     * @brief Returns instance of the `memory_blocks_stack`
     * @return Instance of the `memory_blocks_stack`
     */
    [[nodiscard]] static auto instance() noexcept -> memory_blocks_stack<T> & {
        static memory_blocks_stack<T> instance_;

        return instance_;
    }

private:
    /**
     * @brief Private constructor
     */
    memory_blocks_stack() : stack_pointer_(buffer_.data()) {
    }

    std::array <T, STACK_SIZE> buffer_{};   /**< Internal buffer */
    uint8_t                    *stack_pointer_ = nullptr;      /**< Stack pointer */
};

/**
 * @brief Simple stateful allocator with constant allocation time.
 * @warning There are following limitations for usage:
 *  - Deallocation must be performed in reversed to allocation order
 *  - Allocator doesn't use in thread mode.
 *  - Not production code
 *
 * @tparam T - allocated type
 */
template <class T>
class stack_allocator {
public:
    typedef T value_type; /**< Alias for the type that is managed by allocator */

    /**
     * @brief Constructor
     */
    stack_allocator() : memory_stack_(memory_blocks_stack<T>::instance()) {
    };

    /**
     * @brief Copy constructor is not available for allocator
     */
    template <class U>
    constexpr stack_allocator(const stack_allocator<U> &) = delete;

    /**
     * @brief Assignment operator is not available for singleton object
     */
    stack_allocator &operator=(const stack_allocator) = delete;

    /**
     * @brief Allocated a number of elements of type `T`
     * @param[in] n number of elements
     *
     * @return pointer to allocated memory
     */
    [[nodiscard]] auto allocate(std::size_t n) -> T * {
        T *ptr = memory_stack_.stack_pointer();
        memory_stack_.push(n);

        return ptr;
    }

    /**
     * @brief Frees allocated memory
     * @param[in] p pointer to allocated memory
     * @param[in] n size of allocated memory
     */
    void deallocate(T *p, std::size_t n) noexcept {
        memory_stack_.pop();
    }

private:
    memory_blocks_stack<T> &memory_stack_; /**< Internal allocation buffer */
};

template <class T, class U>
bool operator==(const stack_allocator<T> &, const stack_allocator<U> &) {
    return true;
}

template <class T, class U>
bool operator!=(const stack_allocator<T> &, const stack_allocator<U> &) {
    return false;
}

#endif //QPL_EXAMPLES_HIGH_LEVEL_API_UTILS_STACK_ALLOCATOR_HPP_
