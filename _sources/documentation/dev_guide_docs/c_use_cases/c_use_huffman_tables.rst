 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _huffman-tables-api-label:

Huffman Table Objects
#####################

This page provides general descriptions of the APIs available for
managing a Huffman table object.

The library supports two types of Huffman table: *Deflate* and *Huffman only*.
An object of either type can be created to store compression, decompression,
or both tables internally depending on the user's desired scenario.

The created table is an opaque pointer,
which in addition to user-provided data holds some internal information,
so it is not possible to directly access or modify its fields.
To manage the object, functions for memory allocation, initialization, and
destroy are provided.

Additionally, serialization and deserialization functionalities
are available in order to save state between calls.

Creating Huffman Table
**********************

A Huffman table object can be created as compression, decompression,
or combined table, using the value of the :c:enum:`qpl_huffman_table_type_e` enum.

Users are also required to provide information about execution path
(software or hardware, using :c:enum:`qpl_path_t`)
, and allocator structure (see :c:struct:`allocator_t` for details).

Using one of the :c:func:`qpl_deflate_huffman_table_create`
or :c:func:`qpl_huffman_only_table_create` function
creates a :c:type:`qpl_huffman_table_t` object for *Deflate*
or *Huffman only* respectively, and also allocates memory
required for internal structures.

Example code:

.. code-block:: c

    qpl_huffman_table_t huffman_table;

    qpl_status status = qpl_deflate_huffman_table_create(combined_table_type,
                                                         qpl_path_software,
                                                         {malloc, free},
                                                         &huffman_table);

And similarly for :c:func:`qpl_huffman_only_table_create`.

Initializing Huffman Tables
***************************

Next step is to properly initialize the created table.
There are several ways that this could be done:
initialization with triplets, histogram deflate statistics, or from another table.
Refer to the sections below for implementation and support details.

Huffman Triplets
----------------

An array of the :c:struct:`qpl_huffman_triplet` objects is considered as a
“generic format” Huffman table. *Huffman only* table could be initialized
using the array of triplets. For this the user would need to provide
a pointer to the array and its size together with a pre-allocated Huffman table object.

Example code:

.. code-block:: c

    qpl_huffman_table_t huffman_table;

    qpl_status status = qpl_huffman_only_table_create(compression_table_type,
                                                      qpl_path_hardware,
                                                      {malloc, free},
                                                      &huffman_table);

    qpl_huffman_triplet triplet[256];

    for (i = 0; i < 256; i++) {
        triplet[i].value       = /* <some_value> */;
        triplet[i].code_length = /* <code_length> */;
        triplet[i].code        = /* <some_code> */;
    }

    status = qpl_huffman_table_init_with_triplets(&huffman_table, triplet, 256);

Deflate Tokens Histogram
------------------------

A *Deflate* Huffman table could be built from a deflate tokens histogram
that is defined by the :c:struct:`qpl_histogram` structure.
Histogram structure could be filled using :c:func:`qpl_gather_deflate_statistics` function.
This simply requires the user to then provide a pointer to the complete histogram
and pre-allocated table.

Example code:

.. code-block:: c

    qpl_huffman_table_t huffman_table;

    qpl_status status = qpl_deflate_huffman_table_create(combined_table_type,
                                                         qpl_path_software,
                                                         {malloc, free},
                                                         &huffman_table);

    qpl_histogram deflate_histogram{};

    /* ... Filling deflate histogram using gather_deflate_statistics ... */

    status = qpl_huffman_table_init_with_histogram(huffman_table,
                                                   &deflate_histogram);

Initialization from Other Huffman Table
---------------------------------------

Another way to build a table is to use the function
:c:func:`qpl_huffman_table_init_with_other` to initialize one table with the values from another,
which is also capable of converting a compression table into a decompression representation.

Example code:

.. code-block:: c

    qpl_huffman_table_t other_huffman_table;

    /* ... Creating and filling in other_huffman_table ... */

    qpl_huffman_table_t huffman_table;

    qpl_status status = qpl_deflate_huffman_table_create(decompression_table_type,
                                                         qpl_path_software,
                                                         {malloc, free},
                                                         &huffman_table);

    status = qpl_huffman_table_init_with_other(huffman_table, other_huffman_table);

Destroying Huffman Tables
*************************

In order to destroy a Huffman table object, :c:func:`qpl_huffman_table_destroy` should
be called, that would free up memory for all internal structures as well.

Serializing and Deserializing Huffman Tables
********************************************

    **Note:** Serialization is only supported for `serialization_raw` format.

A Huffman table can be serialized using one of the options in :c:enum:`qpl_serialization_format_e`,
the user should choose based on the desired scenario:
more compact representation vs more straightforward raw format but that could save computing time.

Serialization is a two-step approach.
The first step is to specify serialization options
, and get the size of the table in order to allocate a buffer that would hold
a compact Huffman table representation and could be stored for later use.

Example code:

.. code-block:: c

    qpl_huffman_table_t huffman_table;

    /* ... Creating and initializing table object ... */

    size_t serialized_size;

    status = qpl_huffman_table_get_serialized_size(huffman_table,
                                                   {serialization_raw, 0},
                                                   &serialized_size);

The second step is to provide this pre-allocated buffer and serialize the
Huffman table object.

Example code:

.. code-block:: c

    uint8_t* buffer = (uint8_t*) std::malloc(serialized_size * sizeof(uint8_t*));

    status = qpl_huffman_table_serialize(huffman_table,
                                         buffer,
                                         serialized_size,
                                         {serialization_raw, 0});

Deserialization then would return a valid Huffman table object that would be
created and initialized according to the data stored in the buffer.
Essentially deserialization should be thought of as a combination of create and init
functions for Huffman table. For this providing user-defined allocator is required.

The user is then responsible for destruction of the object returned
by deserialize function via :c:func:`qpl_huffman_table_destroy`.

Example code:

.. code-block:: c

    qpl_huffman_table_t other_huffman_table;
    status = qpl_huffman_table_deserialize(buffer,
                                           serialized_size,
                                           {malloc, free},
                                           &other_huffman_table);

Service Routines
****************

A function :c:func:`qpl_huffman_table_get_type` is available for inquiring a type
of previously created Huffman table according to the values of :c:enum:`qpl_huffman_table_type_e`.




