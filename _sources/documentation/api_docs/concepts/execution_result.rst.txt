 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _execution_result_overview_reference_link:


Execution Result
################


The ``qpl::execution_result`` class represents the result of synchronous
or asynchronous execution.

The library supports two ways to create an object of this class:

1. Constructor for synchronous execution:
   
.. code-block:: cpp

    template<class result_t>
    constexpr explicit execution_result(uint32_t status, result_t result)
            : status_(status),
              result_(std::move(result))
    {
        ...
    }


2. Constructor for asynchronous execution:

.. code-block:: cpp

    template<class result_t>
    constexpr explicit execution_result(const std::shared_future<result_t> &future)
            : future_(future)
    {
    // Empty constructor
    }

where ``result_t`` - type of the kept value.


For more information on ``execution_result`` usage, 
refer to the :ref:`Using execution_result <using_execution_result_reference_link>` chapter.
