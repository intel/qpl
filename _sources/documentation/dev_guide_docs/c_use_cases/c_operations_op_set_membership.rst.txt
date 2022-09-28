 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Set Membership
##############


The ``qpl_set_membership`` operation can be viewed as a generalization of
the ``qpl_scan`` operation. Here, source-2 contains a bit-vector containing 2N
bits, where N is the bit-width of source-1 minus the number of dropped
low and/or high bits. The bits define a set. The output is a bit-vector
whose 1-bits identify which input elements are members of that set.

The number of output bits (i.e. the number of output elements) is the
same as the number of source-1 elements.

.. note:: 
    
    The maximum N for operation is limited by library backend.
