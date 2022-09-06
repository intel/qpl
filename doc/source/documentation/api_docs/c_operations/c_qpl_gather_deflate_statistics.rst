 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _c_qpl_gather_deflate_statistics_reference_link:

qpl_gather_deflate_statistics
#############################


The ``qpl_gather_deflate_statistics`` collects the deflate statistics (a
literal/length tokens histogram and a distance tokens histogram) for a
given vector:

.. code-block:: c

   status = qpl_gather_deflate_statistics(source_ptr, source_length, histogram_ptr, compression_level, execution_path)

where:

::

   source_ptr                   - pointer to source, for which deflate statistics is collected
   source_length                - length of the given vector
   histogram_ptr                - pointer to histgoram to be updated
   compression_level            - compression level of the deflate algorithm (it can be either qpl_default_level, or qpl_high_level)
   execution_path               - execution path parameter (**note:** for now this variable is just a stub, statistics gathering is performed by software only,
                                  this will be fixed in the following release)

This function can be used multiple times for processing one large source
vector with the sequence of smaller buffers, i.e. it updates given
histograms with the new statistics. In this case, further compression
ratio may suffer.
