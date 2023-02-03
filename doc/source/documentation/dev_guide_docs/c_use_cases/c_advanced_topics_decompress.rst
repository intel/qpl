 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Advanced Decompress Features
############################

Normally, the hardware will only write complete qwords (8 bytes) on all,
but a LAST job. If the flag :c:macro:`QPL_FLAG_DECOMP_FLUSH_ALWAYS` is used, then
even on a non-LAST job, all the processed data will be written to the
output.

In case when the application passes in a tiny input buffer, the library
will normally copy the data into internal buffer, and try to combine it
with other small input buffers before submitting it to the hardware.

By default, the library expects a DEFLATE stream to end cleanly during
decompression; otherwise, it reports an error. What this means is that
it expects the stream to end at the end of a block (i.e., with an "EOB"),
and that the block had been marked as a final (i.e., the "b_final" bit is
set in the block header), and furthermore, when it detects a b_final EOB,
it will stop processing.

The application can change this behavior by setting the job
``decomp_end_processing`` field. This holds an enumeration. The
enumeration determines the conditions for stopping the processing, as
well as the expectations for how the stream should end. The table below
shows the meanings of the enumeration values.

+-----------------------------------------+-----------+----------------+
| Value                                   | Stop on   | Expect to End  |
|                                         |           | on             |
+=========================================+===========+================+
| qpl_stop_and_check_for_bfinal_eob       | b_final   | b_final EOB    |
| (default)                               | EOB       |                |
+-----------------------------------------+-----------+----------------+
| qpl_dont_stop_or_check                  |           |                |
+-----------------------------------------+-----------+----------------+
| qpl_stop_and_check_for_any_eob          | EOB       | EOB            |
+-----------------------------------------+-----------+----------------+
| qpl_stop_on_any_eob                     | EOB       |                |
+-----------------------------------------+-----------+----------------+
| qpl_stop_on_bfinal_eob                  | b_final   |                |
|                                         | EOB       |                |
+-----------------------------------------+-----------+----------------+
| qpl_check_for_any_eob                   |           | EOB            |
+-----------------------------------------+-----------+----------------+
| qpl_check_for_bfinal_eob                |           | b_final EOB    |
+-----------------------------------------+-----------+----------------+


In particular, on a non-LAST job (i.e. a job without the LAST flag set),
only the specified “stop” condition will be applied. There will be no
expectation as to the end of that job's buffer. That expectation only
applies for a LAST job. To apply the expectation also to a non-LAST job,
add (or “or”) in the value *qpl_check_on_nonlast_block*.

For example, given a value of *qpl_stop_and_check_for_any_eob*, the
processing stops when EOB on any job appears. The library returns error
if the LAST job does not end with an EOB. In contrast, the values
(*qpl_stop_and_check_for_any_eob* + *qpl_check_for_bfinal_eob*) allow the
library to stop on an EOB on any job. In this case, an error is returned
on any job that is not ended with EOB.

For example, in some non-standard use cases, there may be multiple
blocks, so the processing should not stop on an EOB, and the stream
might be expected to end with an EOB, but that block is not expected to
have the ``b_final`` flag set. In this case, the user can specify
*qpl_check_for_any_eob*. Then the user gets an error if the stream does
not end with an EOB, but the processing will not stop until it reaches
the end of the bit stream.

The ``ignore_start_bits`` and ``ignore_start_bytes`` fields can be used
to start and stop decompression on a bit granularity rather than a byte granularity.

