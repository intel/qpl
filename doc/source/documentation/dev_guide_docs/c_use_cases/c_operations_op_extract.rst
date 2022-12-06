 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Extract
#######


The extract operation outputs input elements whose indices (starting at 0)
fall within the inclusive range defined by ``param_low`` and ``param_high``.
So if the bit width of the output is the same as the bit width of the
input, then the number of output elements should be ``(param_high - param_low + 1)``.

