 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Setting Parser
##############


The Intel® Query Processing Library (Intel® QPL) library 
supports three types of parsers: 

- ``big_endian_packed_array`` 
- ``little_endian_packed_array`` 
- ``parquet_rle``

The examples of how to use them are below.


Big Endian
**********


.. code:: cpp

   auto number_input_elements = get_number_input_elements();

   auto operation = qpl::some_operation::builder()
           .parser<qpl::parsers::big_endian_packed_array>(number_input_elements)
           .build()


Little Endian
*************


.. code:: cpp

   auto number_input_elements = get_number_input_elements();

   auto operation = qpl::some_operation::builder()
           .parser<qpl::parsers::little_endian_packed_array>(number_input_elements)
           .build()


Parquet RLE
***********


         **Note:** For parser ``parquet_rle``, you must specify the
         number of input elements for analytics.

.. code:: cpp

   auto number_input_elements = get_number_input_elements();

   auto operation = qpl::some_operation::builder()
           .parser<qpl::parsers::parquet_rle>(number_input_elements)
           .build()
