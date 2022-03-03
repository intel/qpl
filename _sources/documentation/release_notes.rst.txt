 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Release Notes
#############

v0.1.20-beta
************

**Note**: Initial public release version.

**Known Issues**:

- Analytic operations with compressed input are not supported in the C++ API.
- Scan functionality performance on ``hw-path`` can be unstable.
- ``Dictionary`` mode for ``Compression`` operation is in a prototype stage.
  
  - Stateful execution provides incorrect results in case of Dynamic compression;
  - ``Canned`` mode doesn't supported;

- Library doesn't respect all necessary WQ properties to perform ``Load Balancing``. 
  Performance can be unstable in the high-loaded environment.
- ``qpl_check_job`` and ``qpl_wait_job`` work incorrectly in case of repeated call 
  after operation has been done.
- ``C++ API`` returns unclean code in case if ``hw-path`` initialization fault and 
  on ``wq-busy`` event.
- GCC build warnings.
