<!--
Copyright (C) 2022 Intel Corporation 
SPDX-License-Identifier: MIT
-->

Intel® Query Processing Library (Intel® QPL) Issues Reporting
===============================================================================

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


Introduction
------------------

Intel QPL library has several execution paths and a large amount of different operations and modes,
thus correct issue description is important. Well filled description helps to detect and solve problems faster.

## Steps to Report an Issue
If library behavior is different from expected, the following actions must be done:
1. Research issue.
   1. Recheck an application code and library documentation
   2. Answer the question: What kind of issue is it? (see [Issues Classification](#issues-classification) section)
2. Check if the issue wasn't reported by the Intel QPL team or other contributors. Information about already known issues can be found in:
   1. Release notes: **Known issues** section
   2. Issues tab on GitHub 
4. Submit an issue report if it is a new issue, or subscribe to an already existing issue on GitHub.

## Issues Classification

Issues can be classified depending on the step where they occur:
- Configuration step
- Build step 
- Hardware path initialization step
- Operation execution step

All these classes are described in the following sections:

### Configuration Step Issues

Intel QPL library works with accelerator configuration created by the system administrator.
Many different problems can occur during this step. These problems can affect library work correctness. To avoid these problems follow
instructions for `idxd-config` tool and recommendations in the [Reference manual](QPL_REFERENCE_MANUAL.md):``Library presetting`` section.
>This class of issue is placed out of scope Intel QPL library and can be addressed to `idxd-config` team (https://github.com/intel/idxd-config)

### Build Step Issues
Intel QPL library is distributed as an open-source project. The library can be built using CMake scripts provided in several ways:
- default way without any additional parameters or options 
- library-defined Cmake options declared in the [Reference manual](QPL_REFERENCE_MANUAL.md): ``Specific Build Options`` section
- user-defined specific options

>This class of issue can be resolved by the Intel QPL team.

#### Build Step Issues Report Rules
**Make sure that the issue report contains the following information:**
- Operating system info (OS name)
- Intel QPL version info
- Command line
- CMake version
- CMake log
- Build log

### Initialization Step Issues

Intel QPL library has 2 different status code groups. The first group helps to detect initialization issues.
This issue can be caused by:
- Incorrect configuration that library doesn't support by reason listed in the [Reference manual](QPL_REFERENCE_MANUAL.md):``Library limitations`` section
- Bug in the Library initialization code.

>The second group of problems can be reported to the Intel QPL team. 

#### Initialization Step Issues Report Rules
**Make sure that the issue report contains the following information:**
- Operating system info (OS name, kernel version, accel-config version)
- CPU model
- Intel QPL version info
- API used: C++/C API, Function name, Function prototype
- Instructions to reproduce or a reproducer
- Error description:
  - Returned error code value if none zero
  - Initialization Log (Can be enabled with `-DLOG_HW_INIT=[ON|OFF]` CMake option)
  - Expected behavior description with reference to the behavior described in the Intel QPL documentation

### Operation Execution Step Issues
>This class of issue can be resolved by the Intel QPL team.

Intel QPL library is a complex library with many functions and modes. To debug and detect the particular issue, 
the report needs full information about:
  - Environment
  - Operation properties 
  - Use case

#### Operation Execution Step Issues Report rules
**Make sure that the issue report contains the following information:**
- Operating system info (OS name, kernel version, accel-config version)
- CPU model
- Intel QPL version info
- User-specified CMake options and parameters
- Execution path that was used
- Execution type (asynchronous or synchronous, threading, numa)
- API used: C++/C API, Function name, Function prototype.
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
  - Returned error code value if none zero
  - Execution Log
  - Expected behavior description with reference to the behavior described in the Intel QPL documentation

