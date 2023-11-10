 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _library_testing_reference_link:

Library Testing
###############

Intel® Query Processing Library (Intel® QPL) is distributed with
its own test system based on the GoogleTest framework. The tests
are classified into:

- Functional tests
- Cross tests
- Initialization tests
- Fuzz tests
- Thread tests


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
    use --async=on and --path=hw to specify otherwise

.. note::

   Running functional tests on the hardware path requires first configuring
   Intel® In-Memory Analytics Accelerator (Intel® IAA).
   See :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`.

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

.. note::

   Running cross tests requires first configuring Intel IAA.
   See :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`.


Initialization Tests
********************


Initialization tests validate library initialization code for
the correctness of the hardware path. Initialization tests consist of:

- Python frontend that setup different accelerator configurations before testing
- C++ GoogleTest based backend that runs specific test cases to perform actual testing

To run initialization tests, execute the following commands:

.. code:: shell

   cd <qpl_library>/tools/tests/initialization_tests/test_frontend
   python init_tests.py --test_path=<install_dir>/bin/

To see the full list of other available test options, execute the
following command:

.. code:: shell

   python init_tests.py --help

.. note::

   - Running initialization tests requires first configuring Intel IAA.
     See :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`.

   - To see a detailed initialization log, the library must be built with the
     ``-DLOG_HW_INIT=ON`` CMake option.

   - Initialization tests can be used to validate the existing
     accelerator configuration. To validate configuration correctness,
     execute the following command:

     .. code:: shell

        <install_dir>/bin/init_tests --gtest_filter=*try_init*


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


Thread Tests
************

Thread tests validate library behavior when run in a highly multithreaded environment.
The thread tests detect the number of physical cores on the system, then spawn
an equivalent number of threads and attempt to perform library operations
with each thread. It then ensures that the resulting output is correct. 

To run thread tests, execute the following command:

.. code:: shell

   <install_dir>/bin/tests --dataset=<qpl_library>/tools/testdata/ --gtest_filter=tt_*

Thread tests support both hardware and software paths. To specify the path, use the flag 
`--path=sw` or `--path=hw`. Users can also specify if asynchronous behavior is supported via 
the flag `--async=on` or `--async=off`. By default the path is set to software and behavior
is set to synchronous. 