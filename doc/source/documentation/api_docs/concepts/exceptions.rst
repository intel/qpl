 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Exceptions
##########


The Intel® Query Processing Library (Intel® QPL) high-level API implements 
its own exception system, which is based on ``std::exception``.


Inheritance Hierarchy
*********************


.. figure:: ../../_images/inheritance_hierarchy_1.png
  :scale: 100%
  :align: center

  Exception Inheritance Hierarchy


Exceptions Description
**********************


+-----------------------+-----------------------+-----------------------+
| **Name**              | **Description**       | **Class name**        |
+=======================+=======================+=======================+
| *exception*           | An std::exception     | ``qpl::exception``    |
|                       | analog for the Intel  |                       |
|                       | QPL                   |                       |
+-----------------------+-----------------------+-----------------------+
| *Invalid Argument     | Thrown when           | ``qpl::invalid        |
| Exception*            | initialization of an  | _argument_exception`` |
|                       | operation is          |                       |
|                       | incorrect             |                       |
+-----------------------+-----------------------+-----------------------+
| *Operation Process    | Thrown when Buffer    | ``qpl::operatio       |
| Exception*            | exceeds max size      | n_process_exception`` |
|                       | supported by hardware |                       |
+-----------------------+-----------------------+-----------------------+
| *Memory Overflow      | Thrown in case of an  | ``qpl::memory         |
| Exception*            | exception occurs      | _overflow_exception`` |
|                       | during an operation   |                       |
+-----------------------+-----------------------+-----------------------+
| *Memory Underflow     | Thrown when one of    | ``qpl::memory_        |
| Exception*            | buffers is too small  | underflow_exception`` |
|                       | to finish operation’s |                       |
|                       | execution             |                       |
+-----------------------+-----------------------+-----------------------+
| *Invalid Data         | Thrown in case of     | ``qpl::inv            |
| Exception*            | invalid Compression   | alid_data_exception`` |
|                       | parameter             |                       |
+-----------------------+-----------------------+-----------------------+
| *Invalid Compression  | Thrown when an        | ``qpl:                |
| Parameter Exception*  | operation is          | :invalid_compression_ |
|                       | attempted with        | parameter_exception`` |
|                       | incorrect input       |                       |
|                       | parameters, such as   |                       |
|                       | an incorrect stream   |                       |
|                       | to decompress         |                       |
+-----------------------+-----------------------+-----------------------+
| *Short Destination    | Thrown when the       | ``qpl::short_de       |
| Exception*            | destination buffer is | stination_exception`` |
|                       | too small to finish   |                       |
|                       | the execution. Can be |                       |
|                       | handled by            |                       |
|                       | destination buffer    |                       |
|                       | extending             |                       |
+-----------------------+-----------------------+-----------------------+

For more details on exceptions usage, see the 
:ref:`Using Exceptions <using_exceptions_reference_link>` chapter.
