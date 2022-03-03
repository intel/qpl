 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


qpl_select
##########

The select operation can be considered as a generalization of  the
``qpl_extract`` operation. Here, source-2 is a bit-vector that must have 
at least as many elements as source-1. Those source-1 items that correspond 
to 1-bits in source-2 will be the output.
