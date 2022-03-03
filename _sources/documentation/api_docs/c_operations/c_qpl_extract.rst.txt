 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


qpl_extract
###########


The extract operation outputs input elements whose indices (starting at 1) 
fall within the inclusive range defined by ``param_low`` and ``param_high``.
So the bit width of the output is the same as the bit width of the
input, and the number of output elements should be ``(param_high - param_low + 1)``.

