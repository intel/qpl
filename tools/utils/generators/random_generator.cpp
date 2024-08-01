/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "random_generator.h"

#include <algorithm>
#include <cmath>
#include <iostream>

/* ------ gzRandU ------ */

qpl::test::random_base::random_base(double mpy, double add, uint32_t seed)
    : s1n3(41U), s1n2(18467U), s1n1(6334U), c(1U), s2n1(seed), m_seed(seed), m_mpy(mpy), m_add(add) {
    const double val1 = -m_mpy + m_add;
    const double val2 = +m_mpy + m_add;
    m_valMin          = std::min(val1, val2);
    m_valMax          = std::max(val1, val2);

    i_s1n3 = s1n3;
    i_s1n2 = s1n2;
    i_s1n1 = s1n1;
    for (int i = 0; i < 10; i++)
        operator double(); /// take a running jump
}

void qpl::test::random_base::init(uint32_t seed) {
    s1n3 = i_s1n3;
    s1n2 = i_s1n2;
    s1n1 = i_s1n1;
    c    = 1U;
    s2n1 = seed;
    for (int i = 0; i < 10; i++)
        operator double(); /// take a running jump
}

double qpl::test::random_base::gen() {
    double cnst2in31                = NAN; // exact number for 0.4656613e-9 == 2^-31
    *((uint32_t*)(&cnst2in31) + 0U) = 0x00000000;
    *((uint32_t*)(&cnst2in31) + 1U) = 0x3e000000;

    int s1 = s1n2 - (s1n3 + c);
    if (s1 > 0)
        c = 0;
    else {
        s1 -= 18;
        c = 1;
    }
    s1n3 = s1n2;
    s1n2 = s1n1;
    s1n1 = s1;
    /// int casting is important
    const int s2 = 69069 * s2n1 + 1013904243;
    s2n1         = s2;
    //return m_mpy * 0.4656613e-9 * ( s1 + s2 ) + m_add;
    double result = cnst2in31 * (s1 + s2) * m_mpy + m_add;
    result        = std::max(result, m_valMin);
    result        = std::min(result, m_valMax);
    return result;
}

uint32_t qpl::test::random_base::get_seed() const {
    return s2n1;
}

qpl::test::random_base::operator float() {
    return (float)gen();
}

qpl::test::random_base::operator double() {
    return (double)gen();
}

qpl::test::random_base::operator int8_t() {
    double val = gen();
    val        = (val < 0.) ? (val - 0.5) : (val + 0.5);
    val        = std::min(std::max(val, (double)INT8_MIN), (double)INT8_MAX);

    return (int8_t)val;
}

qpl::test::random_base::operator uint8_t() {
    double val = gen();
    val += 0.5;
    val = std::min(std::max(val, (double)0), (double)UINT8_MAX);

    return (uint8_t)val;
}

qpl::test::random_base::operator int16_t() {
    double val = gen();
    val        = (val < 0.) ? (val - 0.5) : (val + 0.5);
    val        = std::min(std::max(val, (double)INT16_MIN), (double)INT16_MAX);

    return (int16_t)val;
}

qpl::test::random_base::operator uint16_t() {
    double val = gen();
    val += 0.5;
    val = std::min(std::max(val, (double)0), (double)UINT16_MAX);

    return (uint16_t)val;
}

qpl::test::random_base::operator int32_t() {
    double val = gen();
    val        = (val < 0.) ? (val - 0.5) : (val + 0.5);

    int32_t sVal = 0;
    if (val >= (double)INT32_MAX) {
        sVal = INT32_MAX;
    } else if (val < (double)INT32_MIN) {
        sVal = INT32_MIN;
    } else {
        sVal = (int32_t)val;
    }

    return sVal;
}

qpl::test::random_base::operator uint32_t() {
    double val = gen();
    val += 0.5;

    uint32_t uVal = 0U;
    if (val >= (double)UINT32_MAX) {
        uVal = UINT32_MAX;
    } else if (val < (double)0U) {
        uVal = 0U;
    } else if (val >= (double)INT32_MAX) {
        val += (double)INT32_MIN;
        uVal = (uint32_t)val;
        uVal = uVal - INT32_MIN;
    } else {
        uVal = (uint32_t)val;
    }

    return uVal;
}

qpl::test::random_base::operator int64_t() {
    double val = gen();
    val        = (val < 0.) ? (val - 0.5) : (val + 0.5);

    int64_t sVal = 0;
    if (val >= (double)INT64_MAX) {
        sVal = INT64_MAX;
    } else if (val < (double)INT64_MIN) {
        sVal = INT64_MIN;
    } else {
        sVal = (int64_t)val;
    }
    return sVal;
}

qpl::test::random_base::operator uint64_t() {
    double val = gen();
    val += 0.5;

    uint64_t uVal = 0U;
    if (val >= (double)UINT64_MAX) {
        uVal = UINT64_MAX;
    } else if (val < (double)0U) {
        uVal = 0U;
    } else if (val >= (double)INT64_MAX) {
        val += (double)INT64_MIN;
        uVal = (uint64_t)val;
        uVal = uVal - INT64_MIN;
    } else {
        uVal = (uint64_t)val;
    }

    return uVal;
}

const char* qpl::test::random_base::get_reference() {
    return "G.Marsaglia, A.Zaman. Computer in Physics, v8,#1,1994";
}

//std::ostream &operator<<(std::ostream &out, const qpl::test::random_base &instance)
//{
//   out << "mpy="  << instance.m_mpy << " "
//       << "add="  << instance.m_add << " "
//       << "seed=" << instance.m_seed;
//
//   return out;
//}

/* ------ gzRandG ------*/

qpl::test::mean_random::mean_random(double mean, double stdev, uint32_t seed)
    : seed2(1131199209U)
    , seed12(seed)
    , seed13(69069U * seed12 + 1013904243U)
    , seed14(69069U * seed13 + 1013904243U)
    , carry(0xFFFFFFFFU)
    , status(1)
    , m_seed(seed)
    , m_mean(mean)
    , m_stdev(stdev) {
    v1 = v2 = radius = 0.0;
    seed10 = seed11 = 0U;
}

void qpl::test::mean_random::init(uint32_t seed) {
    v1 = v2 = radius = 0.0;
    seed10 = seed11 = 0U;
    seed2           = 1131199209U;
    seed12          = seed;
    seed13          = 69069U * seed12 + 1013904243U;
    seed14          = 69069U * seed13 + 1013904243U;
    carry           = 0xFFFFFFFFU;
    status          = 1;
}

double qpl::test::mean_random::gen() {
    if (status) {
        do { //NOLINT(cppcoreguidelines-avoid-do-while)
            seed2  = 69069U * seed2 + 1013904243U;
            seed11 = seed13 - seed14 + carry;
            carry  = (int)seed11 >> (int)31;
            seed11 -= 18 & carry;
            seed14 = seed12;
            v2     = 0.4656612873077e-9 * (int)(seed11 + seed2);
            seed2  = 69069U * seed2 + 1013904243U;
            seed10 = seed12 - seed13 + carry;
            carry  = (int)seed10 >> (int)31;
            seed10 -= 18 & carry;
            seed13 = seed11;
            seed12 = seed10;
            v1     = 0.4656612873077e-9 * (int)(seed10 + seed2);
            radius = (v1 * v1 + v2 * v2);
        } while (radius >= 1.0 || radius == 0.0);
        status = 0;
        radius = sqrt(-2.0 * log(radius) / radius);
        return (double)(m_mean + (m_stdev * radius * v1));
    } else {
        status = 1;
        return (double)(m_mean + (m_stdev * radius * v2));
    }
}

uint32_t qpl::test::mean_random::get_seed() const {
    return seed10;
}

qpl::test::mean_random::operator char() {
    return (char)gen();
}

qpl::test::mean_random::operator short() {
    return (short)gen();
}

qpl::test::mean_random::operator int() {
    return (int)gen();
}

qpl::test::mean_random::operator float() {
    return (float)gen();
}

qpl::test::mean_random::operator double() {
    return (double)gen();
}

qpl::test::mean_random::operator uint8_t() {
    return (uint8_t)gen();
}

qpl::test::mean_random::operator uint16_t() {
    return (uint16_t)gen();
}

qpl::test::mean_random::operator uint32_t() {
    return (uint32_t)gen();
}

qpl::test::mean_random::operator int8_t() {
    return (int8_t)gen();
}

qpl::test::mean_random::operator int64_t() {
    return (int64_t)gen();
}

qpl::test::mean_random::operator uint64_t() {
    return (uint64_t)gen();
}

//std::ostream &operator<<(std::ostream &out, const qpl::test::mean_random &instance)
//{
//   out << "mean="  << (float) instance.m_mean << " "
//       << "stdev=" << (float) instance.m_stdev << " "
//       << "seed="  << instance.m_seed;
//
//   return out;
//}

void qpl::test::random::set_range(double min, double max) {
    m_mpy    = (max - min) / 2.0;
    m_add    = (max + min) / 2.0;
    m_valMin = std::min(-m_mpy + m_add, +m_mpy + m_add);
    m_valMax = std::max(-m_mpy + m_add, +m_mpy + m_add);
}
