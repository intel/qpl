 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Asynchronous Execution
######################


An operation or a chain can be performed asynchronously using
``qpl::submit()`` function for operation submission.


.. note::
    
    Asyncronous execution in C++ API is work in progress. Current
    implementation uses ``qpl::execute()`` function.


``qpl::submit()`` returns the ``qpl::execution_result<uint32_t, qpl::sync>``
object before the operation is completed.


.. note::
    
    Currently, ``qpl::submit`` always returns ``qpl::execution_result<uint32_t,
    qpl::sync>`` that holds a number of elements processed to the destination.
