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
   :project: Intel QPL

.. doxygenfunction:: qpl_huffman_only_table_create
    :project: Intel QPL

.. doxygenfunction:: qpl_huffman_table_destroy
    :project: Intel QPL

Initialization of Huffman table
-------------------------------

.. doxygenfunction:: qpl_huffman_table_init_with_triplets
    :project: Intel QPL

.. doxygenfunction:: qpl_huffman_table_init_with_histogram
    :project: Intel QPL

.. doxygenfunction:: qpl_huffman_table_init_with_other
    :project: Intel QPL

Serialization APIs
------------------

.. doxygenfunction:: qpl_huffman_table_get_serialize_size
    :project: Intel QPL

.. doxygenfunction:: qpl_huffman_table_serialize
    :project: Intel QPL

.. doxygenfunction:: qpl_huffman_table_deserialize
    :project: Intel QPL

.. doxygenfunction:: qpl_huffman_table_get_type
    :project: Intel QPL

Types
*****

.. doxygentypedef:: qpl_huffman_table_t
   :project: Intel QPL

Enums
*****

.. doxygenenum:: qpl_huffman_table_type_e
   :project: Intel QPL

.. doxygenenum:: qpl_path_t
   :project: Intel QPL
   :outline:

.. doxygenenum:: qpl_serialization_format_e
   :project: Intel QPL

Structures
**********

.. doxygenstruct:: allocator_t
   :project: Intel QPL
   :members:

.. doxygenstruct:: qpl_huffman_triplet
   :project: Intel QPL
   :members:

.. doxygenstruct:: qpl_histogram
   :project: Intel QPL
   :members:

.. doxygenstruct:: serialization_options_t
   :project: Intel QPL
   :members: