 .. ***************************************************************************
 .. * Copyright (C) 2023 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _multiple_jobs_reference_link:

Compression and Decompression across Multiple Jobs
##################################################

Compression and decompression of a large stream can be accomplished through a
sequence of jobs. For example, an application that was compressing a large file
might call Intel® Query Processing Library (Intel® QPL) repeatedly
with 64 KB input buffers, until the end of the file was reached.
In this case, the first job is specified with the flag :c:macro:`QPL_FLAG_FIRST`
in :c:member:`qpl_job.flags`, the middle jobs do not need additional flags,
and the last job is specified with the flag :c:macro:`QPL_FLAG_LAST`.

In comparison, if a single job is used to compress the stream, then both
:c:macro:`QPL_FLAG_FIRST` and :c:macro:`QPL_FLAG_LAST` flags must be set.

.. attention::
   The same :c:struct:`qpl_job` object can be reused for multiple jobs.
   In this case, it is user responsibility to ensure that the job is reset correctly.
   :c:func:`qpl_init_job` shouldn't be called for this purposes,
   it should only be called when a new :c:struct:`qpl_job` object is allocated.

A compression job will never reference data from an earlier job. This implies that
submitting, for example, 10 jobs of 1,000 bytes each will generate different output than
submitting one job with all 10,000 bytes. In general, submitting very small buffers
will result in a worse compression ratio than submitting fewer large buffers.

.. _library_multiple_jobs_limitations_link:

Limitations
***********

Currently, Intel® QPL has a few limitations when using multiple jobs:

- The current implementation does not support canned compression/decompression
  across multiple jobs, thus canned compression/decompression jobs must specify
  the flags :c:macro:`QPL_FLAG_FIRST` | :c:macro:`QPL_FLAG_LAST`.

- All jobs in a sequence will be completed on the same execution path. If the application
  specifies ``Auto Path`` for the sequence, only the first job can initiate a host fallback.
  This means that if the first job runs successfully on the accelerator, the following
  jobs will not fall back to the host even if they fail to execute on the accelerator.
  The error will be returned to the user. If the first job falls back to the host,
  the following jobs in the sequence will also run on the host.
