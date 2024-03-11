/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "iostream"
#include "algorithm"
#include "format_generator.hpp"
#include "source_provider.hpp"
#include "random_generator.h"
#include "qpl_test_environment.hpp"

namespace qpl::test
{
    auto source_provider::get_source() -> std::vector<uint8_t>
    {
        std::vector<uint8_t> result_vector;

        if (m_parser == qpl_p_parquet_rle) {
            result_vector = generate_prle_stream();
        } else {
            result_vector = format_generator::generate_uint_bit_sequence(m_number_of_elements,
                                                                         m_bit_width,
                                                                         m_seed,
                                                                         m_parser == qpl_p_le_packed_array);
        }

        return result_vector;
    }

    auto source_provider::get_prle_header_bytes_size(uint32_t count) -> uint32_t
    {
        uint32_t bit_count = 0; // number of bits required to store count value

        for (; count != 0; bit_count++)
        {
            count >>= 1;
        }

        uint32_t header_bytes = 1;

        header_bytes += (bit_count > 6) ? 1 : 0;
        header_bytes += (bit_count > 13) ? 1 : 0;
        header_bytes += (bit_count > 20) ? 1 : 0;

        return header_bytes;
    }

    void source_provider::store_prle_header(std::vector<uint8_t>::iterator &source_it,
                                            uint32_t count,
                                            prle_encoding_t prle_encoding)
    {
        uint64_t buffer             = 0;
        int32_t  bits_in_buffer     = 0;
        uint64_t mask               = 0;
        uint32_t header_bytes       = get_prle_header_bytes_size(count);

        if (prle_encoding_t::parquet == prle_encoding) // store encoding type
        {
            buffer = (1ULL << 0);
        }

        mask = MASK_SIX_LOW_BITS;

        // store first 6 bits of count with 1 bit shift, because 1st byte contains encoding type information
        buffer |= (uint64_t(count) & mask) << 1;

        if (header_bytes > 1) // check if additional bytes required to store count
        {
            // set last bit to indicate that the following byte is still header
            buffer |= (1UL << PRLE_COUNT_FOLLOWING_BYTE_BITS);
        }

        *source_it = uint8_t(buffer); // store first header byte
        header_bytes--;
        source_it++;

        buffer         = 0;
        bits_in_buffer = 0;
        mask           = MASK_SEVEN_LOW_BITS;
        count >>= PRLE_COUNT_FIRST_BYTE_BITS;

        while (header_bytes > 0) // store the rest of count bits
        {
            buffer |= (uint64_t(count) & mask) << bits_in_buffer;

            count >>= PRLE_COUNT_FOLLOWING_BYTE_BITS;
            bits_in_buffer += PRLE_COUNT_FOLLOWING_BYTE_BITS;
            header_bytes--;

            if (header_bytes > 0)
            {
                buffer |= (1ULL << bits_in_buffer);
                bits_in_buffer++;
            }

            if (bits_in_buffer >= BIT_BUF_LEN_HALF)
            {
                auto source32_ptr = reinterpret_cast<uint32_t *>(&(*source_it));
                *source32_ptr = (uint32_t) buffer;
                source_it += sizeof(uint32_t);
                buffer >>= BIT_BUF_LEN_HALF;
                bits_in_buffer -= BIT_BUF_LEN_HALF;
            }
        }

        while (0 < bits_in_buffer)
        {
            *source_it = (uint8_t) buffer;
            source_it++;
            buffer >>= BYTE_BIT_LENGTH;
            bits_in_buffer -= BYTE_BIT_LENGTH;
        }
    }

    void source_provider::store_parquet_group(std::vector<uint8_t>::iterator &source_it,
                                              std::vector<uint8_t>::iterator source_end_it,
                                              uint32_t count,
                                              const std::vector<uint32_t>& elements) const
    {
        uint64_t mask           = (1ULL << m_bit_width) - 1;
        uint64_t buffer         = 0;
        int32_t  bits_in_buffer = 0;

        for (auto element : elements)
        {
            buffer |= (uint64_t(element) & mask) << bits_in_buffer;
            bits_in_buffer += m_bit_width;

            if (bits_in_buffer >= BIT_BUF_LEN_HALF)
            {
                auto source32_ptr = reinterpret_cast<uint32_t *>(&(*source_it));

                if (source_it >= source_end_it)
                {
                    throw std::out_of_range("Not enough elements in the temporary buffer while generating PRLE stream");
                }

                *source32_ptr = (uint32_t) buffer;
                source_it += sizeof(uint32_t);
                buffer >>= BIT_BUF_LEN_HALF;
                bits_in_buffer -= BIT_BUF_LEN_HALF;
            }
        }

        while (0 < bits_in_buffer)
        {
            if (source_it >= source_end_it)
            {
                throw std::out_of_range("Not enough elements in the temporary buffer while generating PRLE stream");
            }

            *source_it = (uint8_t) buffer;
            source_it++;
            buffer >>= BYTE_BIT_LENGTH;
            bits_in_buffer -= BYTE_BIT_LENGTH;
        }
    }

    void source_provider::store_rle(std::vector<uint8_t>::iterator &source_it,
                                    uint32_t count,
                                    uint32_t element) const
    {
        uint64_t mask           = (1ULL << m_bit_width) - 1;
        uint64_t buffer         = 0;
        int32_t  bits_in_buffer = 0;

        buffer |= (uint64_t(element) & mask) << bits_in_buffer;
        bits_in_buffer += m_bit_width;

        while (0 < bits_in_buffer)
        {
            *source_it = (uint8_t) buffer;
            source_it++;
            buffer >>= BYTE_BIT_LENGTH;
            bits_in_buffer -= BYTE_BIT_LENGTH;
        }
    }

    auto source_provider::generate_prle_stream() -> std::vector<uint8_t>
    {
        const uint32_t minimal_vector_size = 100U;
        uint32_t       initial_vector_size = (m_number_of_elements) * (m_bit_width / BYTE_BIT_LENGTH + 1);
        initial_vector_size = (initial_vector_size < minimal_vector_size) ? minimal_vector_size : initial_vector_size;
        std::vector<uint8_t> result_vector(
                initial_vector_size * 3); // Temporary fix, TODO:: change memory allocation logic

        uint64_t      max_input_value   = (1ULL << m_bit_width) - 1;
        qpl::test::random num_generator(0, max_input_value, m_seed);
        qpl::test::random count_generator(1, m_number_of_elements, m_seed);

        std::vector<uint32_t> octa_group(PRLE_OCTA_GROUP_SIZE, 0);
        auto bit_width = static_cast<uint8_t>(m_bit_width);

        auto source_it = result_vector.begin();
        *source_it = bit_width; // store bitwidth of the elements to output stream
        source_it++;

        uint32_t elements_remain = m_number_of_elements; // we want to m_number_of_elements to be generated and stored

        uint32_t count       = 0;       // count variable in prle stream
        uint32_t rle_element = 0; // element to be run length encoded

        while (elements_remain > 0)
        {
            // check if it's possible to store an octa group and generate it's in half of cases
            if (elements_remain >= PRLE_OCTA_GROUP_SIZE && 0 == (uint32_t(num_generator) % 2))
            {
                // number of generated elements is up to elements_remain
                count = (uint32_t(count_generator) % (elements_remain / PRLE_OCTA_GROUP_SIZE));
                count = count / 2 + 1; // reduce number of elements
                source_provider::store_prle_header(source_it, count, prle_encoding_t::parquet);

                std::generate(octa_group.data(), octa_group.data() + PRLE_OCTA_GROUP_SIZE,
                              [&num_generator]()
                              {
                                  return static_cast<uint32_t>(num_generator);
                              });

                for (uint32_t i = 0; i < count; i++) // generate parquet group to result vector
                {
                    uint32_t number_of_attempts = 0U;

                    try
                    {
                        number_of_attempts++;
                        store_parquet_group(source_it, result_vector.end(), count, octa_group);
                    }
                    catch (const std::exception &e)
                    {
                        if (number_of_attempts > 3)
                        {
                            std::cerr << e.what() << '\n';
                            break;
                        }
                        result_vector.insert(result_vector.end(), result_vector.size(), 0U);
                        store_parquet_group(source_it, result_vector.end(), count, octa_group);
                    }

                    elements_remain -= PRLE_OCTA_GROUP_SIZE;
                }
            }
            else // store run length encoding element otherwise
            {
                count       = uint32_t(count_generator) % elements_remain + 1;
                rle_element = uint32_t(num_generator);
                store_prle_header(source_it, count, prle_encoding_t::run_length_encoding);
                store_rle(source_it, count, rle_element);
                elements_remain -= count;
            }
        }

        result_vector.erase(source_it, result_vector.end());

        return result_vector;
    }

    auto source_provider::generate_expand_rle_prle_stream() -> std::vector<uint8_t>
    {
        uint32_t             initial_vector_size = (m_number_of_elements) * (m_bit_width / BYTE_BIT_LENGTH + 2);
        std::vector<uint8_t> result_vector(initial_vector_size);

        uint64_t      max_count_expand_rle_value = (1ULL << 2);
        qpl::test::random num_generator(1, max_count_expand_rle_value, m_seed);
        qpl::test::random count_generator(1, m_number_of_elements, m_seed);

        std::vector<uint32_t> octa_group(PRLE_OCTA_GROUP_SIZE, 0);

        auto bit_width = static_cast<uint8_t>(m_bit_width);

        auto source_it = result_vector.begin();
        *source_it = bit_width; // store bitwidth of the elements to output stream
        source_it++;

        uint32_t elements_remain = m_number_of_elements; // we want to m_number_of_elements to be generated and stored

        uint32_t count = 0; // count variable in prle stream

        bool accumulate_values = (m_bit_width == 32);

        m_count_number_expand_rle = 0;

        if (accumulate_values)
        {
            count = 1;
            store_prle_header(source_it, count, prle_encoding_t::run_length_encoding);
            store_rle(source_it, count, 0);
            elements_remain -= count;
        }

        count = 0;

        while (elements_remain > 0)
        {
            if (elements_remain >= PRLE_OCTA_GROUP_SIZE && uint32_t(num_generator) % 2 == 0 ||
                (accumulate_values && elements_remain >= PRLE_OCTA_GROUP_SIZE))
            {
                uint32_t octa_groups_remain = elements_remain / PRLE_OCTA_GROUP_SIZE;
                octa_groups_remain += (0 == elements_remain % PRLE_OCTA_GROUP_SIZE) ? 0 : 1;
                count                       = (uint32_t(count_generator) %
                                               (octa_groups_remain)); // number of generated elements is up to elements_remain
                count =
                        count / 2 + 1;                                      // reduce number of elements

                if (accumulate_values)
                    count = 1;

                source_provider::store_prle_header(source_it, count, prle_encoding_t::parquet);

                for (uint32_t i = 0; i < count; i++) // generate parquet group to result vector
                {
                    if (accumulate_values)
                    {
                        octa_group[0] = m_count_number_expand_rle;

                        for (auto octa_group_it = octa_group.begin() + 1;
                             octa_group_it < octa_group.end(); octa_group_it++)
                        {
                            *octa_group_it = *(octa_group_it - 1) + uint32_t(num_generator);
                        }

                        m_count_number_expand_rle = octa_group.back();
                    }
                    else
                    {
                        std::generate(octa_group.data(), octa_group.data() + PRLE_OCTA_GROUP_SIZE,
                                      [&num_generator]()
                                      {
                                          return static_cast<uint32_t>(num_generator);
                                      });

                        for (auto element : octa_group)
                        {
                            m_count_number_expand_rle += element * count;
                        }
                    }

                    uint32_t number_of_attempts = 0U;

                    try
                    {
                        number_of_attempts++;
                        store_parquet_group(source_it, result_vector.end(), count, octa_group);
                    }
                    catch (const std::exception &e)
                    {
                        if (number_of_attempts > 3)
                        {
                            std::cerr << e.what() << '\n';
                            break;
                        }
                        result_vector.insert(result_vector.end(), result_vector.size(), 0U);
                        store_parquet_group(source_it, result_vector.end(), count, octa_group);
                    }

                    elements_remain -= PRLE_OCTA_GROUP_SIZE;
                }
            }
            else
            {
                uint32_t rle_element;

                if (accumulate_values)
                {
                    count                     = 1;
                    rle_element               = m_count_number_expand_rle + uint32_t(num_generator);
                    m_count_number_expand_rle = rle_element;
                }
                else
                {
                    count       = uint32_t(count_generator) % elements_remain + 1;
                    rle_element = uint32_t(num_generator);
                    m_count_number_expand_rle += rle_element * count;
                }

                store_prle_header(source_it, count, prle_encoding_t::run_length_encoding);
                store_rle(source_it, count, rle_element);
                elements_remain -= count;
            }
        }

        result_vector.erase(source_it, result_vector.end());

        return result_vector;
    }

    auto generate_mask(uint32_t number_of_elements) -> std::vector<uint8_t> {
        source_provider mask_generator(number_of_elements,
                                       1U,
                                       util::TestEnvironment::GetInstance().GetSeed(),
                                       qpl_p_le_packed_array);

        return mask_generator.get_source();
    }

    auto source_provider::get_counter_source_expand_rle(uint16_t prologue) -> std::vector<uint8_t>
    {
        if (!(m_bit_width == 8U || m_bit_width == 16U || m_bit_width == 32U))
        {
            throw std::exception();
        }

        std::vector<uint8_t> result_source;

        if (m_parser == qpl_p_parquet_rle)
        {
            result_source = generate_expand_rle_prle_stream();
        }
        else
        {
            uint32_t              max_value = (1ULL << 2);
            qpl::test::random     num_generator(1, max_value, m_seed);
            std::vector<uint32_t> temporary_source_vector(m_number_of_elements, 0);

            if (m_bit_width == 32U)
            {
                for (auto it = temporary_source_vector.begin() + 1; it < temporary_source_vector.end(); it++)
                {
                    auto new_value = (uint32_t) num_generator;
                    *it = new_value + *(it - 1);
                }
            }
            else
            {
                std::generate(temporary_source_vector.begin(), temporary_source_vector.end(),
                              [&num_generator]()
                              {
                                  return static_cast<uint32_t>(num_generator);
                              });
            }

            if (m_bit_width == 32U)
            {
                m_count_number_expand_rle = temporary_source_vector.back();
            }
            else
            {
                for (auto times_repeated : temporary_source_vector)
                {
                    m_count_number_expand_rle += times_repeated;
                }
            }

            result_source.resize(prologue);

            result_source = format_generator::push_back_uint_vector(result_source,
                                                                    temporary_source_vector,
                                                                    m_bit_width,
                                                                    m_parser == qpl_p_le_packed_array);
        }

        return result_source;
    }

    auto source_provider::get_count_expand_rle_value() const -> uint32_t
    {
        return m_count_number_expand_rle;
    }
}
