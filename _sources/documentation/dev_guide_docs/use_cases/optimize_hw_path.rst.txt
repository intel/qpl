 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _hw_path_optimization_reference_link:


Hardware Path Optimizations
###########################


The following section contains information about optimizations for
hardware path supported by Intel® Query Processing Library (Intel® QPL).


Multi-Descriptor Processing
***************************


The library is able to split ``Set membership`` and ``Scan`` operations
onto several sub-tasks and performing them in paralleled mode. In other
words, if the following conditions are met:

- This is ``Set membership`` or ``Scan`` (``Scan range``) operation. 
- Nominal bit array output, i.e. no output modifications. 
- Input data size is at least 32kB large.
- Input is in Little- or Big-Endian format (no encoded input support).
- No more than 1 work queue per 1 device is configured.

Then the operation is auto-paralleled on the library level during
execution on hardware path. This is intended to receive better
performance when processing large streams.
