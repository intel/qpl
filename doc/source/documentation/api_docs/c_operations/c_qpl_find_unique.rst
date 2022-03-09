 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


qpl_find_unique
###############


This operation is in some sense the inverse of the ``qpl_set_membership`` 
operation. Conceptually, it starts with a bit-vector of size 2N, 
where N is the bit-width of source-1 minus the number of dropped
low and/or high bits. The output bit vector starts with all zero values. 
As it reads the elements from source-1, the hardware sets those bits 
corresponding to the values from source-1. At the end, the 1-bits 
in the bit vector specify all of the values seen from source-1. 
This bit vector is the output.


.. note:: 
    
    The maximum N for operation is limited by library backend.
