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
might call Intel QPL repeatedly with 64 KB input buffers, until the end of the
file was reached. In this case, the first job is specified with the flag
:c:macro:`QPL_FLAG_FIRST` in :c:member:`qpl_job.flags`, the middle jobs do not need
additional flags, and the last job is specified with the flag :c:macro:`QPL_FLAG_LAST`.
If a single job is used to compress the stream, then both flags should be specified.
The same :c:struct:`qpl_job` object can be reused for multiple jobs. And
:c:func:`qpl_init_job` should not be called in between jobs. (:c:func:`qpl_init_job`
should be called only after a new :c:struct:`qpl_job` object is allocated)

.. attention::
   The current implementation does not support canned compression/decompression
   across multiple jobs, thus canned compression/decompression jobs must specify
   the flags :c:macro:`QPL_FLAG_FIRST` | :c:macro:`QPL_FLAG_LAST`.

Intel QPL does not save history state between jobs. This means that one compression
job will never reference data from an earlier job. This implies that submitting,
for example, 10 jobs of 1,000 bytes each will generate different output than
submitting one job with all 10,000 bytes. In general, submitting very small buffers
will result in a worse compression ratio than submitting fewer large buffers.