 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Library Testing
###############


Intel® Query Processing Library (Intel® QPL) is distributed with 
its own test system based on the GoogleTest framework. The tests
are classified into: 

- Functional tests 
- Cross tests 
- Initialization tests
- Fuzz tests


Functional Tests
****************

Functional tests provide functional validation of Intel QPL APIs. These tests are
further divided into four groups: 

- Algorithmic tests (``ta_*``) serve to find errors that reflect logic 
  errors in data processing on correct data. 
- Bad argument tests (``tb_*``) verify the code path for invalid arguments.
- Negative tests (``tn_*``) serve to find errors that reflect lack of 
  control over the input data format. 
- Thread tests (``tt_*``) detect out of order read/write
  operations for common structures by different threads.


The tests can be launched using a single executable ``<install_dir>/bin/tests``.
To run all the functional tests, execute the following command:

.. code:: shell

   <install_dir>/bin/tests --dataset=<qpl_library>/tools/testdata/

To run, for example, algorithmic tests only, use:

.. code:: shell

   <install_dir>/bin/tests --dataset=<qpl_library>/tools/testdata/ --gtest_filter=ta_*


Other available test options include:

- ``--seed=<random_seed>`` - Specifies the random seed used in generating some
  testing data (timer value is used by default).
- ``--path=[hw|sw]`` - Runs functional tests on the hardware path (``sw``, or
  software path, is used by default).
- ``--async=[on|off]`` - Tests asynchronous API (``off``, or synchronous, is used
  by default). See :ref:`asynchronous_execution_reference_link`.

.. note::

   Running functional tests on the hardware path requires first configuring
   Intel® In-Memory Analytics Accelerator (Intel® IAA).
   See :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`.


Cross Tests
***********


Cross tests provide validation of: 

- Input/output stream format compatibility between hardware
  and software paths for the same Intel QPL operation. 
  Especially for compression/decompression functionality. 
- Aggregates, checksums equality between hardware and software paths 
  for the same Intel QPL operation.

Cross tests intend to assure that software and hardware paths can be
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
the correctness of hardware path. Initialization tests consist of: 

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

- ``<qpl_library>/build/tools/tests/fuzzing/high-level-api/`` 
- ``<qpl_library>/build/tools/tests/fuzzing/low-level-api/``

To run fuzz tests, specify the maximum run time using ``-max_total_time=<seconds>``,
for example:

.. code:: shell

   <qpl_library>/build/tools/tests/fuzzing/low-level-api/deflate_dynamic_default_nodict_fuzz_test -max_total_time=15
