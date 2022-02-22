<!--
Copyright (C) 2022 Intel Corporation 
SPDX-License-Identifier: MIT
-->

Intel® Query Processing Library (Intel® QPL) Testing
=================================================

Notices and Disclaimers
=======================

No license (express or implied, by estoppel or otherwise) to any intellectual
property rights is granted by this document.

Intel disclaims all express and implied warranties, including without
limitation, the implied warranties of merchantability, fitness for a particular
purpose, and non-infringement, as well as any warranty arising from course of
performance, course of dealing, or usage in trade. This document contains
information on products, services and/or processes in development. All
information provided here is subject to change without notice. Contact your
Intel representative to obtain the latest forecast, schedule, specifications and
roadmaps. The products and services described may contain defects or errors
which may cause deviations from published specifications. Current characterized
errata are available on request. Intel, the Intel logo, Intel Atom, Intel Core
and Xeon are trademarks of Intel Corporation in the U.S. and/or other countries.
\*Other names and brands may be claimed as the property of others. Microsoft,
Windows, and the Windows logo are trademarks, or registered trademarks of
Microsoft Corporation in the United States and/or other countries. Java is a
registered trademark of Oracle and/or its affiliates.

© Intel Corporation.

This software and the related documents are Intel copyrighted materials, and
your use of them is governed by the express license under which they were
provided to you ("License"). Unless the License provides otherwise, you may
not use, modify, copy, publish, distribute, disclose or transmit this software
or the related documents without Intel's prior written permission. This software
and the related documents are provided as is, with no express or implied
warranties, other than those that are expressly stated in the License.

Overview
-------------

Intel QPL is distributed with its own test system based on the google-test framework. The testing is performed with the following testing groups:
 - Functional tests
 - Cross tests 
 - Initialization tests
 - Fuzzing tests


### Functional Tests

Functional testing provides the fundamental validation for both Public APIs and some subset of tests for internal Intel QPL entities.
Tests are segregated onto several test groups:
 + Algorithmic tests (`ta_<test_suite>.<test>`) serve to find errors that reflect logic errors in data processing on correct data.
 + Bad argument tests (`tb_<test_suite>.<test>`) validate code on preventive identification of errors in the function arguments.
 + Negative tests (`tn_<test_suite>.<test>`) serve to find errors that reflect lack of control over the input data format.
 + Thread tests (`tt_<test_suite>.<test>`) -  detect of out of order read/write operations for common structures by different threads.

You can find the tests in `<install_dir>/bin`.
To test the library, execute the following command:

```shell
./tests
```

***Note 1:*** Some tests require special data files (for example, all tests related to compression use the standard de-facto data set Calgary corpus)
therefore you should either put these files to the same place/path where the test is available or use the `--dataset` switch
for example:
```
./tests --dataset=<qpl_library>/tools/testdata/
``` 

To see the full list of other available test options, execute the following command:

```shell
./tests --help
```


### Cross Tests
Cross tests provide validation of:
 - Input/output stream format compatibility between `Hardware` and `Software paths` for the same Intel QPL operation. 
   Especially for compression/decompression functionality.
 - Aggregates, checksums equality between Hardware` and `Software paths` for the same Intel QPL operation.

Cross tests indicate that `Software` and `Hardware` paths can be exchanged in the application code without behavior change. 

You can find the cross_tests in `<install_dir>/bin`.
To test the library, execute the following command:

```shell
./cross_tests
```

***Note 1:*** Some tests require special data files (for example, all tests related to compression use the standard de-facto data set Calgary corpus)
therefore you should either put these files to the same place/path where the test is available or use the `--dataset` switch
for example:
```
./cross_tests --dataset=<qpl_library>/tools/testdata/
``` 

To see the full list of other available test options, execute the following command:

```shell
./cross_tests --help
```


### Initialization Tests

Initialization tests validate library initialization code for `Hardware Path` correctness. Initialization tests consist of:
- Python frontend that setup different Accelerator configurations before testing
- C++ google-test based backend that runs specific test cases to perform actual testing

To test the library, execute the following commands:

```shell
cd <library_dir>/tools/tests/initialization_tests/test_frontend
python init_tests.py --test_path=<install_dir>/bin/init_tests
```

To see the full list of other available test options, execute the following commands:

```shell
cd <library_dir>/tools/tests/initialization_tests/test_frontend
python init_tests.py --help
```

***Note 1:*** Initialization tests can be used to validate the existing accelerator configuration.
To validate configuration correctness, execute the following commands:

```shell
cd <install_dir>/bin/
./init_tests --gtest_filter=*try_init*
```

To see a detailed initialization log, the library must be built with the `-DLOG_HW_INIT=ON` CMake option.


### Fuzzing Tests

Fuzz testing is an automated software testing technique that attempts to find hackable software bugs by randomly
feeding invalid and unexpected inputs and data into a computer program in order to find coding errors and security loopholes.

QPL fuzz testing is based on [LibFuzzer tool](https://llvm.org/docs/LibFuzzer.html) and requires separate build with 
the `-DLIB_FUZZING_ENGINE=ON` CMake option and `clang` as compiler.

Fuzzing tests are not installed into `<install_dir>/bin` but available in the:
- `<cmake build>/tools/tests/fuzzing/high-level-api/` 
- `<cmake build>/tools/tests/fuzzing/low-level-api/`
