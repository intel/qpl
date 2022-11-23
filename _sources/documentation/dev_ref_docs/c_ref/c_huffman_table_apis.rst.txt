 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Huffman Table APIs
##################

Functions
*********

Creation and Destruction of Huffman table
-----------------------------------------

.. doxygenfunction:: qpl_deflate_huffman_table_create
   :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_huffman_only_table_create
    :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_huffman_table_destroy
    :project: Intel(R) Query Processing Library

Initialization of Huffman table
-------------------------------

.. doxygenfunction:: qpl_huffman_table_init_with_triplets
    :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_gather_deflate_statistics
    :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_huffman_table_init_with_histogram
    :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_huffman_table_init_with_other
    :project: Intel(R) Query Processing Library

Serialization APIs
------------------

.. doxygenfunction:: qpl_huffman_table_get_serialized_size
    :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_huffman_table_serialize
    :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_huffman_table_deserialize
    :project: Intel(R) Query Processing Library

.. doxygenfunction:: qpl_huffman_table_get_type
    :project: Intel(R) Query Processing Library

Types
*****

.. doxygentypedef:: qpl_huffman_table_t
   :project: Intel(R) Query Processing Library

Enums
*****

.. doxygenenum:: qpl_huffman_table_type_e
   :project: Intel(R) Query Processing Library

.. doxygenenum:: qpl_serialization_format_e
   :project: Intel(R) Query Processing Library

Structures
**********

.. doxygenstruct:: qpl_huffman_triplet
   :project: Intel(R) Query Processing Library
   :members:

.. doxygenstruct:: qpl_histogram
   :project: Intel(R) Query Processing Library
   :members:

.. doxygenstruct:: serialization_options_t
   :project: Intel(R) Query Processing Library
   :members:
