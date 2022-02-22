/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef HW_PATH_OWN_HW_CHECKERS_H_
#define HW_PATH_OWN_HW_CHECKERS_H_

#include "hw_devices.h"

/* ------ Bad argument checkers ------ */

/* Define NULL pointer value */
#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


#define HW_IMMEDIATELY_RET(expression, status) \
if((expression)) { \
    return status; \
}

#define HW_IMMEDIATELY_RET_NULLPTR(expression) \
if(NULL == (expression)) { \
    return QPL_STS_NULL_PTR_ERR; \
}

/* ------ State checkers ------ */


#endif //HW_PATH_OWN_HW_CHECKERS_H_
