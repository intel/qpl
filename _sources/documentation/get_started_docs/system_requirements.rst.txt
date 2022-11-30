 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _system_requirements_reference_link:

System Requirements
###################


Software Path Requirements
**************************

On Linux* OS, the accelerator configuration library ``libaccel-config.so``
(see :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`)
must be placed in ``/usr/lib64/`` to build and run
the Intel® Query Processing Library (Intel® QPL), even if only the Software Path is used.

| **Minimum requirements**
| x86-64 CPU with Intel® Advanced Vector Extensions 2 support
  (Intel® microarchitecture code name Broadwell).

| **Recommended requirements for better performance**
| x86-64 CPU with Intel® Advanced Vector Extensions 512 support
  (Intel® microarchitecture code name Skylake (Server) processor or higher).


.. _system_requirements_hw_path_reference_link:

Hardware Path Requirements
**************************

- **Linux* OS**

  - The accelerator configuration library ``libaccel-config.so``
    (see :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`)
    must be placed in ``/usr/lib64/``.
- Linux kernel version 5.18 or later. Public versions of Linux
  kernels can be found here: https://www.kernel.org.
- Virtualization technology for directed I/O (VT-d) is enabled through the BIOS menu.
- Sudo privileges are required to configure accelerators
  (see :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`)
  and run hardware path applications.
- The POSIX thread (pthread) library is required if the library is built with option ``-DDWQ_SUPPORT=ON``
  (see :ref:`building_library_build_options_reference_link`).

.. _accelerator_configuration_reference_link:

Accelerator Configuration
=========================


Intel® In-Memory Analytics Accelerator (Intel® IAA) devices can be configured with
``libaccel-config`` library, which can be found here: https://github.com/intel/idxd-config.

The Intel® QPL hardware path requires ``libaccel-config``
library version 3.2 or higher.
Refer to `accel-config releases <https://github.com/intel/idxd-config/releases>`__ for the
latest version.

.. note::

  By default, Intel® QPL uses ``Block On Fault`` feature
  required to handle page faults on the Intel® IAA side. The
  ``block on fault`` flag must be set with ``accel-config`` for each
  device. Performance can be increased if an application performs
  ``pre-faulting`` by own. In this case, the ``block on fault`` flag is
  not required and ``Block On Fault`` feature must be disabled with the
  CMake build option ``-DBLOCK_ON_FAULT=OFF`` (see the
  :ref:`building_library_build_reference_link` and
  :ref:`building_library_build_options_reference_link` sections).

