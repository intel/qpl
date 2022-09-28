 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _hw_path_optimizations_reference_link:

Hardware Path Optimizations
###########################


The following section contains information about optimizations for
hardware path supported by Intel® Query Processing Library (Intel® QPL).


Multi-Descriptor Processing
***************************


The library can split ``qpl_set_membership`` and ``qpl_scan`` operations onto
several sub-tasks and perform them in paralleled mode. In other words,
if the following conditions are met:

-  This is ``qpl_set_membership`` or ``qpl_scan`` (``qpl_scan_range``) operation.
-  Nominal bit array output, i.e. no output modifications.
-  Input data size is at least 32kB large.
-  Input is in Little- or Big-Endian format (no encoded input support).
-  Aggregates and checksums are disabled via appropriate flags.
-  No more than 1 work queue per 1 device configured.
-  Operation should be executed with synchronous interface
   ``qpl_execute_job(...)``.

**Note**: the last limitation is temporary due to library refactoring.

Then the operation is auto-paralleled on the library level during
execution on hardware path. This is intended to receive better
performance when processing large streams.

