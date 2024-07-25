/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __TSRAND_H__
#define __TSRAND_H__
#include <cstdint>
#include <string>

namespace qpl::test {
constexpr uint32_t DEFAULT_SEED = 1131199209U;

/// random generator with the range mpy*(-1..1)+add
class random_base {
public:
    explicit random_base(double mpy = 1, double add = 0, uint32_t seed = DEFAULT_SEED);

    void init(uint32_t seed = DEFAULT_SEED);

    uint32_t get_seed() const;

    static const char* get_reference();

    explicit operator float();

    explicit operator double();

    explicit operator int8_t();

    explicit operator uint8_t();

    explicit operator int16_t();

    explicit operator uint16_t();

    explicit operator int32_t();

    explicit operator uint32_t();

    explicit operator int64_t();

    explicit operator uint64_t();

    //friend std::ostream &operator<<(std::ostream &out, const random_base &instance);

protected:
    uint32_t i_s1n3, i_s1n2, i_s1n1;
    uint32_t s1n3, s1n2, s1n1, c, s2n1;
    uint32_t m_seed;
    double   m_add;
    double   m_mpy;
    double   m_valMax, m_valMin;

    double gen();
};

/// random generator with the range [min..max]
class random : public random_base {
public:
    explicit random(double min = 0., double max = 1., uint32_t seed = DEFAULT_SEED)
        : random_base((max - min) / 2.0, (max + min) / 2.0, seed) {}

    void set_range(double min, double max);
};

/// random generator with Normal distribution
class mean_random {
public:
    mean_random(double mean, double stdev, uint32_t seed = DEFAULT_SEED);

    void init(uint32_t seed = DEFAULT_SEED);

    uint32_t get_seed() const;

    explicit operator char();

    explicit operator short();

    explicit operator int();

    explicit operator float();

    explicit operator double();

    explicit operator uint8_t();

    explicit operator uint16_t();

    explicit operator uint32_t();

    explicit operator int8_t();

    explicit operator int64_t();

    explicit operator uint64_t();

    //friend std::ostream &operator<<(std::ostream &out, const mean_random &instance);

protected:
    uint32_t seed2, seed10, seed11, seed12, seed13, seed14, carry;
    int32_t  status;
    uint32_t m_seed;
    double   m_mean, m_stdev;
    double   v1, v2, radius;

    double gen();
};
} // namespace qpl::test
#endif // __TSRAND_H__
