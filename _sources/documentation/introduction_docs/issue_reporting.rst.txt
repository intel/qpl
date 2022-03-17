 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Issue Reporting
###############


Introduction
************


The Intel® Query Processing Library (Intel® QPL) library has several 
execution paths and a large amount of different operations and modes, 
thus correct issue description is important. Well filled description 
helps to detect and solve problems faster.


Steps to Report an Issue
************************


If the library behavior is different from expected, the following
actions must be done: 

1. Research issue. 
2. Recheck an application code and `library documentation <https://intel.github.io/qpl/index.html>`__.
3. Answer the question: What kind of issue is it? (see :ref:`Issues Classification <issue_classification_link>`). 
4. Check if the issue wasn't reported by the Intel QPL team or other contributors. 
   Information about already known issues can be found in: 
   
   1. Release notes: `Known Issues <https://intel.github.io/qpl/documentation/release_notes.html>`__ section. 
   2. Issues tab on GitHub (see `Issues tab <https://github.com/intel/qpl/issues>`__). 

5. Submit an issue report if it is a new issue, or subscribe to an already existing issue on GitHub.


.. _issue_classification_link:

**Issue Classification**


Issues can be classified depending on the step where they occur: 

- Configuration step 
- Build step 
- Hardware path initialization step
- Operation execution step

All these classes are described in the following sections:


Configuration Step Issue
************************


Intel QPL library works with accelerator configuration created by the
system administrator. Many problems can occur during this step. These
problems can affect library work correctness. To avoid these problems,
follow instructions for ``idxd-config`` tool and recommendations in the 
:ref:`Library Presetting and Building 
<library_presetting_and_building_reference_link>` section.

This class of issue is placed out of scope Intel QPL library and can be 
addressed to ``idxd-config`` team (https://github.com/intel/idxd-config).


Build Step Issue
****************


Intel QPL library is distributed as an open-source project. The library
can be built using CMake scripts provided in several ways: 

- default way without any additional parameters or options 
- library-defined Cmake options declared in the :ref:`Specific Build Options: 
  <cpecific_build_options_reference_link>` section
- user-defined specific options

This class of issue can be resolved by the Intel QPL team.


Build Step Issue Report Rules
=============================


**Make sure that the issue report contains the following information:**

- Operating system info (OS name) 
- Intel QPL version info 
- Command line 
- CMake version 
- CMake log 
- Build log


Initialization Step Issue
*************************


Intel QPL library has 2 different status code groups. The first group
helps to detect initialization issues. This issue can be caused by: 

- Incorrect configuration that library does not support
  by reason listed in the :ref:`Library Limitations 
  <library_limitations_reference_link>` section.
- Bug in the Library initialization code.

The second group of problems can be reported to the Intel QPL team.


Initialization Step Issue Report Rules
======================================


**Make sure that the issue report contains the following information:**

- Operating system info (OS name, kernel version, accel-config version)
- CPU model 
- Intel QPL version info 
- API used: C++/C API, Function name, Function prototype 
- Instructions to reproduce or a reproducer 
- Error description: 
  
  - Returned error code value if none zero
  - Initialization Log (Can be enabled with ``-DLOG_HW_INIT=[ON|OFF]`` CMake option)
  - Expected behavior description with reference to the behavior described in the 
    `Intel QPL documentation <https://intel.github.io/qpl/index.html>`__


Operation Execution Step Issue
******************************


This class of issue can be resolved by the Intel QPL team.

Intel QPL library is a complex library with many functions and modes. To
debug and detect the particular issue, the report needs full information
about: 

- Environment 
- Operation properties 
- Use case


Operation Execution Step Issue Report Rules
===========================================


**Make sure that the issue report contains the following information:**

- Operating system info (OS name, kernel version, accel-config version)
- CPU model 
- Intel QPL version info 
- User-specified CMake options and parameters 
- Execution path that was used 
- Execution type (asynchronous or synchronous, threading, numa) 
- API used: C++/C API, Function name, Function prototype 
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
  - Expected behavior description with reference to the behavior described in 
    the `Intel QPL documentation <https://intel.github.io/qpl/index.html>`__
