 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _library_testing_reference_link:

Library Testing
###############

.. note::

   For the latest information on the state of Intel® Query Processing Library (Intel® QPL) testing,
   refer to the "Known Limitations" section in the `Release Notes <https://github.com/intel/qpl/releases>`__.

.. attention::

   Running library testing on the ``Hardware Path`` (or running Cross tests) requires first configuring
   Intel® In-Memory Analytics Accelerator (Intel® IAA).
   See :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`.

   To test with the dataset provided in the Intel QPL repository under ``tools/testdata``,
   user needs to configure the Intel IAA with a maximum transfer size of 2 GB.
   This is done by setting the ``max_transfer_size`` parameter to 2147483648.
   By default, the maximum transfer size is set to 2 MB or 2097152 bytes.

   If it is not set correctly,
   user may encounter an error code :c:macro:`QPL_STS_TRANSFER_SIZE_INVALID` returned by Intel QPL.

Intel® Query Processing Library (Intel® QPL) is distributed with
its own test system based on the GoogleTest framework. The tests
are classified into:

- Functional tests
- Cross tests
- Fuzz tests

Functional Tests
****************

Functional tests provide functional validation of Intel QPL APIs. These tests are
further divided into four groups:

- Algorithmic tests (``ta_*``) serve to find errors that reflect logic
  errors in data processing on correct data.
- Bad argument tests (``tb_*``) verify the code path for invalid arguments.
  They perform null input checks, buffer overlap checks,
  and input size checks to ensure the safety of all operations.
- Negative tests (``tn_*``) serve to find errors that reflect lack of
  control over the input data format. These tests also check for cases
  that would lead to output overflow, and ensure proper error codes
  are thrown before overflow occurs.
- Thread tests (``tt_*``) validate library behavior when run in a highly multithreaded environment.
  The thread tests detect the number of physical cores on the system, then spawn
  an equivalent number of threads and attempt to perform library operations
  with each thread. It then ensures that the resulting output is correct.

The tests can be launched using a single executable ``<install_dir>/bin/tests``.
To run all the functional tests, execute the following command:

.. code:: shell

   <install_dir>/bin/tests --dataset=<qpl_library>/tools/testdata/

To run specific tests only, use the GoogleTest option ``--gtest_filter``. For example, to run algorithmic tests only, use:

.. code:: shell

   <install_dir>/bin/tests --dataset=<qpl_library>/tools/testdata/ --gtest_filter=ta_*


In order to get the list of GoogleTest test options, use the following command:

.. code:: shell

   <install_dir>/bin/tests --help

To see the full list of other available test options specific to the library
(e.g., execution path, synchronous or asynchronous mode), use the following command:

.. code:: shell

   <install_dir>/bin/tests --qpl-tests-help

.. note::

    software_path and synchronous execution mode is used for testing by default,
    use ``--async=on`` and ``--path=hw`` (or ``--path=auto``) to specify otherwise.

Cross Tests
***********

Cross tests provide validation of:

- Input/output stream format compatibility between the hardware
  and software paths for the same Intel QPL operation.
  Especially for compression/decompression functionality.
- Aggregates, checksums equality between the hardware and software paths
  for the same Intel QPL operation.

Cross tests intend to assure that the software and hardware paths can be
exchanged in the application code without behavior change.

The tests can be launched using a single executable ``<install_dir>/bin/cross_tests``.
To run cross tests, execute the following command:

.. code:: shell

   <install_dir>/bin/cross_tests --dataset=<qpl_library>/tools/testdata/

Fuzz Tests
**********

Fuzz testing is an automated software testing technique that attempts to
find hackable software bugs by randomly feeding invalid and unexpected
inputs and data into a computer program in order to find coding errors
and security loopholes.

Intel QPL fuzz testing is based on `LibFuzzer
tool <https://llvm.org/docs/LibFuzzer.html>`__ and requires a separate
build using the Clang compiler (version 12.0.1 or higher) with the
``-DLIB_FUZZING_ENGINE=ON`` CMake option.

Fuzz tests are not installed into ``<install_dir>/bin/`` but available
in:

- ``<qpl_library>/build/tools/tests/fuzzing/low-level-api/``

To run fuzz tests, specify the maximum run time using ``-max_total_time=<seconds>``,
for example:

.. code:: shell

   <qpl_library>/build/tools/tests/fuzzing/low-level-api/deflate_dynamic_default_nodict_fuzz_test -max_total_time=15
