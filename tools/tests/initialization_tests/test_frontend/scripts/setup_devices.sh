# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

SCRIPT_NAME=$(basename $BASH_SOURCE)
SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
DEVICE_PREFIX=iax

if [[ "$1" == "" ]]; then
    echo "Usage: ${SCRIPT_NAME} <device config> <wq config>"
    echo "<device config> - none, one, all"
    echo "<wq config>     - none, one, all_in_one, all"
    echo "Specifc device and wq is randomly chousen for option <one>"
    exit 1
fi

Exec() {
    echo "$@"
    $@
}

# Try to use python3 explicitly
Python() {
    if ! command -v python3 &> /dev/null; then
        Exec python $@
    else
        Exec python3 $@
    fi
}

backup_lib()
{
    if ! [ -f $1.bak ]; then
        Exec cp $1 $1.bak
        if [ $? != 0 ]; then
            echo "Error: cannot backup $1"
            exit 1
        fi
    else
        echo "$1 backup already exists"
    fi
}

restore_lib()
{
    if [ -f $1.bak ]; then
        Exec mv $1.bak $1
        if [ $? != 0 ]; then
            echo "Error: cannot restore $1"
            exit 1
        fi
    
    elif ! [ -f $1 ]; then
        echo "Error: $1 library not found"
        exit 1
        
    else
        echo Found: $1
    fi
}

echo "Restore libaccel-config"
echo "======================="
ACCELCONF_SO=$(ldconfig -p | grep libaccel-config.so$ | sed -e 's/^.*=> //')
ACCELCONF_SO_VER1=${ACCELCONF_SO}.1
ACCELCONF_SO_VER2=${ACCELCONF_SO_VER1}.0.0

restore_accelconf()
{
    restore_lib ${ACCELCONF_SO_VER2}
    ln -rf -s ${ACCELCONF_SO_VER2} ${ACCELCONF_SO_VER1}
    ln -rf -s ${ACCELCONF_SO_VER2} ${ACCELCONF_SO}
}

restore_accelconf

echo ""
echo "Find devices"
echo "============"
DEVICES=($(find /sys/bus/ | grep -o -P "${DEVICE_PREFIX}\d+" | sort -u))
NUM_DEVICES=${#DEVICES[@]}
echo "${NUM_DEVICES} devices: ${DEVICES[*]}"

# All wars are -1 for inclusive ranges from 0
NUM_DEVICES=$(expr $NUM_DEVICES - 1)
NUM_WQS=7
NUM_GRPS=3

if [ "$1" == "prolog" ]; then
    echo ""
    echo "Backup configuration"
    echo "===================="
    
    if ! [ -f "./conf_backup.json" ]; then
        Exec accel-config save-config -s ./conf_backup.json
    else
        echo "Accelerator config backup already exists"
    fi
    
    exit 0
    
elif [ "$1" == "epilog" ]; then
    echo ""
    echo "Rstore configuration"
    echo "===================="
    
    if [ -f "./conf_backup.json" ]; then
        Python ${SCRIPT_DIR}/accel_conf.py --load ./conf_backup.json
        rm ./conf_backup.json

    else
        echo "Error: no backup found"
        exit 1
    fi
        
    exit 0
fi

echo ""
echo "Reset devices"
echo "============="
for DEVICE in "${DEVICES[@]}"; do
    Exec accel-config disable-device ${DEVICE}
done

configure_wqs()
{
    DEV=$1
    
    for WQ in $(seq 0 $NUM_WQS); do
        GRP=$(echo $(($RANDOM % ${NUM_GRPS})))
        
        accel-config config-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ} --wq-size 16
        accel-config config-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ} --mode shared
        accel-config config-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ} --group-id ${GRP}
        accel-config config-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ} --type user
        accel-config config-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ} --priority 10
        accel-config config-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ} --threshold 10
        accel-config config-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ} -n app1shared
        
        accel-config config-engine ${DEVICE_PREFIX}${DEV}/engine${DEV}.${WQ} --group-id ${GRP}
    done
}

if [ "$1" == "all" ]; then
    echo ""
    echo "Setup devices (all)"
    echo "==================="
    
    for DEV in $(seq 0 $NUM_DEVICES); do
        DEV=$(echo ${DEVICES[${DEV}]} | sed -e 's/^[A-Za-z]*//')
        
        configure_wqs ${DEV}
        Exec accel-config enable-device ${DEVICE_PREFIX}${DEV}

        if [ "$2" == "all" ]; then
            for WQ in $(seq 0 $NUM_WQS); do
                Exec accel-config enable-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ}
            done
        fi
    done
    
    if [ "$2" == "all_in_one" ]; then
        DEV=$(echo $(($RANDOM % ${NUM_DEVICES})))
        DEV=$(echo ${DEVICES[${DEV}]} | sed -e 's/^[A-Za-z]*//')
    
        for WQ in $(seq 0 $NUM_WQS); do
            Exec accel-config enable-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ}
        done
    elif [ "$2" == "one" ]; then
        DEV=$(echo $(($RANDOM % ${NUM_DEVICES})))
        DEV=$(echo ${DEVICES[${DEV}]} | sed -e 's/^[A-Za-z]*//')
        WQ=$(echo $(($RANDOM % ${NUM_WQS})))
    
        Exec accel-config enable-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ}
    fi
    
    chmod 777 /dev/${DEVICE_PREFIX}/*
    
elif [ "$1" == "one" ]; then
    DEV=$(echo $(($RANDOM % ${NUM_DEVICES})))
    DEV=$(echo ${DEVICES[${DEV}]} | sed -e 's/^[A-Za-z]*//')
    WQ=$(echo $(($RANDOM % ${NUM_WQS})))
    
    echo ""
    echo "Setup devices (one)"
    echo "==================="
    configure_wqs ${DEV}
    
    Exec accel-config enable-device ${DEVICE_PREFIX}${DEV}
    
    if [ "$2" == "one" ]; then
        Exec accel-config enable-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ}
    elif [ "$2" == "all_in_one" || "$2" == "all" ]; then
        for WQ in $(seq 0 $NUM_WQS); do
            Exec accel-config enable-wq ${DEVICE_PREFIX}${DEV}/wq${DEV}.${WQ}
        done
    fi
    
fi

if [ -d /dev/${DEVICE_PREFIX} ]; then
    chmod 777 /dev/${DEVICE_PREFIX}/*
fi
