# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

SCRIPT_NAME=$(basename $BASH_SOURCE)
SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
DEVICE_PREFIX=iax

# Try to use python3 explicitly
Python() {
    if ! command -v python3 &> /dev/null; then
        python $@
    else
        python3 $@
    fi
}

Python ${SCRIPT_DIR}/accel_conf.py --filter ${DEVICE_PREFIX} $@
