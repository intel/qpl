# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

SCRIPT_NAME=$(basename $BASH_SOURCE)

if [[ "$1" == "" ]]; then
    echo "Usage: ${SCRIPT_NAME} <test case>"
    echo "Cases:"
    echo "    no_libaccel"
    echo "    no_devices"
    echo "    no_wqs"
    echo "    no_access"
    echo "    fault_accfg_new"
    exit 1
fi

if [ "$1" == "no_libaccel" ]; then
    # Reset devices
    source ./scripts/setup_devices.sh none none

    echo ""
    echo "Disable libaccel-config.so"
    echo "=========================="
    
    backup_lib ${ACCELCONF_SO_VER2}
    rm ${ACCELCONF_SO_VER2}

elif [ "$1" == "no_devices" ]; then
    echo ""
    echo "Disable all devices"
    echo "==================="

    # Reset devices
    source ./scripts/setup_devices.sh none none

elif [ "$1" == "no_wqs" ]; then
    echo ""
    echo "Disable all wqs"
    echo "==============="

    # Reset devices
    source ./scripts/setup_devices.sh all none

elif [ "$1" == "no_access" ]; then
    echo ""
    echo "Disable device access"
    echo "====================="

    # Reset devices
    source ./scripts/setup_devices.sh all all
    chmod 000 /dev/${DEVICE_PREFIX}/*

elif [ "$1" == "fault_accfg_new" ]; then
    echo ""
    echo "Set fault lib"
    echo "============="
    
    # Reset devices
    source ./scripts/setup_devices.sh none none

    backup_lib ${ACCELCONF_SO_VER2}

    ACCELCONF_NAME=$(basename ${ACCELCONF_SO_VER2})
    ACCELCONF_FAULT="${ACCELCONF_NAME}_fault_accfg_new"
    ACCELCONF_DIR=$(dirname ${ACCELCONF_SO_VER2})
    Exec cp ${SCRIPT_DIR}/${ACCELCONF_FAULT} ${ACCELCONF_DIR}/${ACCELCONF_NAME}
fi
