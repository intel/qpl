/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <stddef.h>
#include <stdint.h>

#ifndef HW_PATH_OWN_DEFLATE_H_
#define HW_PATH_OWN_DEFLATE_H_

#define DYNAMIC_HDR           2U                          /**< @todo */
#define DYNAMIC_HDR_SIZE      3U                          /**< @todo */
#define MAX_CODE_LEN          15U                         /**< @todo */
#define MAX_HEAP              286U                        /**< @todo */
#define HEAP_SIZE             (3U * MAX_HEAP + 1U)        /**< @todo */
#define NODE_START            (HEAP_SIZE - 1U)            /**< @todo */
#define FREQ_SHIFT            32U                         /**< @todo */
#define DEPTH_SHIFT           24U                         /**< @todo */
#define DEPTH_MASK            0x7F                        /**< @todo */
#define FREQ_MASK_HI          0xFFFFFFFF80000000          /**< @todo */
#define DEPTH_MASK_HI         (DEPTH_MASK << DEPTH_SHIFT) /**< @todo */
#define DEPTH_1               (1U << DEPTH_SHIFT)         /**< @todo */
#define MAX_BL_CODE_LEN       7U                          /**< @todo */
#define BITBUF2_BUFFER_SLOP   7U                          /**< @todo */
#define STOP_CHECK_RULE_COUNT 7U                          /**< @todo */

/* ------ Operation Properties ------ */
/**
 * @name Deflate operation states
 * @anchor HW_DEFLATE_STATES
 * @todo
 * @{
 */
#define DEF_STATE_TERM     8U /**< @todo // processing terminated   */
#define DEF_STATE_FINAL    4U /**< @todo // in final block          */
#define DEF_STATE_SB       2U /**< @todo // in stored block         */
#define DEF_STATE_HDR      1U /**< @todo // looking at block header */
#define DEF_STATE_LL_TOKEN 0U /**< @todo // looking at block header */
/** @} */

#endif //HW_PATH_OWN_DEFLATE_H_
