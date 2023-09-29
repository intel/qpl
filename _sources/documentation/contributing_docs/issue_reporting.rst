 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Issue Reporting
###############


The Intel® Query Processing Library (Intel® QPL) library has several
execution paths and various modes and operations, thus specific issue
description is important to help detect and solve problems faster.


Steps to Report an Issue
************************


If the library behavior is different from expected, follow the steps below to
determine if it is a real and new issue and how to submit it:

1. Double-check your application code and `library documentation <https://intel.github.io/qpl/index.html>`__.
2. Root cause the issue.
3. Check if the issue was already reported by the Intel QPL team or other contributors.
   Information about already known issues can be found in:

   - The section of known issues in `Release Notes <https://github.com/intel/qpl/releases>`__.
   - `Issues on Github <https://github.com/intel/qpl/issues>`__.

4. If it is a new issue, classify the issue (see :ref:`Issue Classification <issue_classification_link>`).
5. Submit the issue on GitHub.


.. _issue_classification_link:

Issue Classification
********************


Issues can be classified based on when they occur:

- Configuration step
- Build step
- Hardware path initialization step
- Operation execution step

These classes are described in the following sections.


Configuration Step Issue
========================


Intel QPL library hardware path requires Intel® In-Memory Analytics Accelerator devices
to be configured properly before application execution. Problems occurred during
accelerator configuration can affect the correctness of Intel QPL library.
To avoid these problems, follow instructions of ``idxd-config`` tool.

This class of issues is out of scope of Intel QPL library and should be
reported to ``idxd-config`` team (https://github.com/intel/idxd-config).


Build Step Issue
================


Intel QPL library can be built using the provided CMake scripts in several ways:

- The default build without any additional parameters or options
- Using the library-defined CMake options
  (see :ref:`building_library_build_options_reference_link`)
- Using user-defined options

This class of issues can be resolved by the Intel QPL team.

**Make sure that your issue report contains the following information:**

- OS name
- Intel QPL version
- Command line
- CMake version
- CMake log
- Build log


Initialization Step Issue
=========================


Intel QPL library has 2 different status code groups. The first group
helps to detect initialization issues. This issue can be caused by:

- Incorrect configuration that library does not support
  by reason listed in the :ref:`library_limitations_reference_link` section.
- Bug in the Library initialization code.

The second group of problems can be reported to the Intel QPL team.

**Make sure that your issue report contains the following information:**

- Operating system info

  - OS name
  - Kernel version
  - accel-config library version
- CPU model
- Intel QPL version
- API used, incl. function name and a list of input parameters
- Instructions to reproduce or a reproducer
- Error description:

  - Returned error code value
  - Initialization Log (can be enabled with the CMake build option ``-DLOG_HW_INIT=ON``)
  - Expected behavior description with reference to the behavior described in the
    `Intel QPL documentation <https://intel.github.io/qpl/index.html>`__


Operation Execution Step Issue
==============================


This class of issues can be resolved by the Intel QPL team.

**Make sure that your issue report contains the following information:**

- Operating system info

  - OS name
  - Kernel version
  - accel-config library version
- CPU model
- Intel QPL version
- User-specified CMake options and parameters
- Execution path
- Execution type (asynchronous or synchronous, threading, numa)
- API used, incl. function name and a list of input parameters
- Operation description:

  - Operation type
  - Operation mode
  - Input data format for operation
  - Output data format for operation
  - Operation flags
  - Other operation properties

- Instructions to reproduce or a reproducer
- Dataset to reproduce if applicable
- Error description:

  - Returned error code value
  - Execution Log
  - Expected behavior description with reference to the behavior described in
    the `Intel QPL documentation <https://intel.github.io/qpl/index.html>`__
