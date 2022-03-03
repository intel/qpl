 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _using_compression_streams_reference_link:


Using Compression Streams
#########################


See common methods for compression streams:

.. code:: cpp

   /**
    * @brief Method that returns an iterator pointing to the first element in the buffer
    */
   auto begin() noexcept -> uint8_t *;

   /**
    * @brief Method that returns an iterator pointing to the past-the-end element in the buffer
    */
   auto end() noexcept -> uint8_t *;

   /**
    * @brief Method that returns the number of elements in the buffer
    */
   auto size() noexcept -> size_t;


Deflate Stream
**************


See two main methods for processing input data for the
``qpl::deflate_stream``:

.. code:: cpp

   template<class input_iterator_t>
   auto push(const input_iterator_t &source_begin,
             const input_iterator_t &source_end) -> deflate_stream &;

   template<class input_iterator_t>
   void flush(const input_iterator_t &source_begin,
              const input_iterator_t &source_end) noexcept;

Using the ``push()`` method, ``qpl::deflate_stream``
accepts new chunks of the input stream for compression.

After all the chunks are delivered, using the ``flush()`` method, it
is necessary to send the very last chunk of the input stream for
processing.


Simple Use Example
==================


.. code:: cpp

   #include <qplhl/results/deflate_stream.hpp>

   int main()
   {
       std::vector<uint8_t> source(1000, 5);
       std::vector<uint8_t> destination(500, 4);

       auto deflate_operation = qpl::deflate_operation();
       auto deflate_stream    = qpl::deflate_stream(deflate_operation,
                                                    destination.size());
       
       deflate_stream.push(source.begin() + 0, source.begin() + 500)
                    .flush(source.begin() + 500, source.begin() + 1000);

       return 0;
   }

The destination buffer might become full and further decompression
will be impossible.

Then, it is possible to catch exceptions of a specific type
``qpl::short_destination_exception`` and reassign the new output
buffer using:

.. code:: cpp

   void resize(size_t new_size) noexcept;


If ``new_size`` is smaller, than then current buffer
size, the content is reduced to its first ``new_size``
elements, removing those beyond (and destroying them).

If ``new_size`` is equal to the current buffer size,
no copying is performed.


Example of Use with a Reset of Buffer
=====================================


.. code:: cpp

   #include <qplhl/results/inflate_stream.hpp>

   int main()
   {
       std::vector<uint8_t> source(1000, 5);
       std::vector<uint8_t> destination(500, 4);
       std::vector<uint8_t> short_destination(10, 4);

       auto deflate_operation = qpl::deflate_operation();
       auto deflate_stream    = qpl::deflate_stream(deflate_operation,
                                                    short_destination.size());
       
       try
       {
           deflate_stream.push(source.begin() + 0, source.begin() + 500);
       } catch (qpl::short_destination_exception &e)
       {
           deflate_stream.reset(destination.size());
           deflate_stream.push(source.begin() + 0, source.begin() + 500);
       }

       deflate_stream.flush(source.begin() + 500, source.begin() + 1000);

       return 0;
   }


Inflate Stream
**************


The main method for processing input data for the
``qpl::inflate_stream``:

.. code:: cpp

   template<class output_iterator_t>
   auto extract(const output_iterator_t &destination_begin,
                const output_iterator_t &destination_end) -> inflate_stream &;

Using the ``extract()`` method described earlier,
``qpl::inflate_stream`` decompresses the data into the specified buffer.


Simple Use example
==================


.. code:: cpp

   #include <qplhl/results/inflate_stream.hpp>

   int main()
   {
       std::vector<uint8_t> reference(1000, 5);
       std::vector<uint8_t> destination(500, 4);

       auto inflate_operation = qpl::inflate_operation();
       auto inflate_stream    = qpl::inflate_stream(inflate_operation,
                                                    destination.begin(),
                                                    destination.end());
       
       inflate_stream.extract(reference.begin() + 0, reference.begin() + 500)
                .extract(reference.begin() + 500, reference.begin() + 1000);

       return 0;
   }
