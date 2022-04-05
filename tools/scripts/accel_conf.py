# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

import argparse
import os
import subprocess
import sys;
import json;

def run_cmd(cmd, args=[''], is_root=False):
    cmd = ['sudo', cmd] + args if is_root else [cmd] + args
    p = subprocess.Popen(cmd,
                         universal_newlines=True,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    out, err = p.communicate()
    return [p.returncode, out, err]


def accel_get_active_dvices():
    ret, out, err = run_cmd(cmd="accel-config", args=["list"])
    if ret:
        return [ret, err, out]
    else:
        return [ret, err, json.loads(out)]
    
def accel_get_all_dvices():
    ret, out, err = run_cmd(cmd="accel-config", args=["list", "-i"])
    if ret:
        return [ret, err, out]
    else:
        return [ret, err, json.loads(out)]

def accel_load_config(config_file, is_root=False):
    ret, out, err = run_cmd(cmd="accel-config", args=["load-config", "-v", "-c", config_file], is_root=is_root)
    return [ret, err, out]

def accel_disable_device(device, is_root=False):
    ret, out, err = run_cmd(cmd="accel-config", args=["disable-device", "-v", device], is_root=is_root)
    return [ret, err, out]
    
def accel_enable_device(device, is_root=False):
    ret, out, err = run_cmd(cmd="accel-config", args=["enable-device", "-v", device], is_root=is_root)
    return [ret, err, out]
    
def accel_enable_wq(device, wq, is_root=False):
    ret, out, err = run_cmd(cmd="accel-config", args=["enable-wq", "-v", device + "/" + wq], is_root=is_root)
    return [ret, err, out]

def accel_set_block_on_fault(device, wq, bof_flag, is_root=False):
    ret, out, err = run_cmd(cmd="accel-config", args=["config-wq", device + "/" + wq, "-b", str(int(bof_flag))], is_root=is_root)
    return [ret, err, out]


def get_aggregated(dev_filter):
    numas   = 0
    devices = 0
    wqs     = 0
    engines = 0

    ret, err, devices_list = accel_get_active_dvices()
    devices_list.sort(key=lambda x: x["numa_node"])

    numa  = devices_list[0]["numa_node"]
    numas = 1
    numa_comp = False

    devices_names = {}
    device1 = devices_list[0]["dev"]
    for device in devices_list:
        if not dev_filter in device["dev"]:
            continue
    
        if numa != device["numa_node"]:
            numas += 1
            numa = device["numa_node"]
            numa_comp = True

        if numa_comp:
            # TODO
            print("Warning: non-uniform numas configuration")
        else:
            devices += 1
            groups = 0
            group_idx = 0
            group_active = -1
            for group in device["groups"]:
                if "grouped_workqueues" in group:
                    groups += 1
                    if group_active < 0:
                        group_active = group_idx
                group_idx += 1

            if groups > 1:
                print("Warning: multiple groups for device: " + device["dev"])
            if groups == 0:
                print("Warning: no groups for device: " + device["dev"])
            if wqs:
                if wqs != len(device["groups"][group_active]["grouped_workqueues"]) or engines != len(device["groups"][group_active]["grouped_engines"]):
                    print("Warning: non-uniform devices configuration for devices: " + device1 + " and " + device)
            else:
                wqs     = len(device["groups"][group_active]["grouped_workqueues"])
                engines = len(device["groups"][group_active]["grouped_engines"])
    return [numas, devices, wqs, engines]

def get_devices_short():
    ret, err, devices_list = accel_get_active_dvices()

    device_dict = {}
    for device in devices_list:
        if "iax" in device["dev"]:
            device_gen = "iax"
        elif "dsa" in device["dev"]:
            device_gen = "dsa"
        
        if not device_gen in device_dict:
            device_dict[device_gen] = {}
        if not device["numa_node"] in device_dict[device_gen]:
            device_dict[device_gen][device["numa_node"]] = {}
        if not device["dev"] in device_dict[device_gen][device["numa_node"]]:
            device_dict[device_gen][device["numa_node"]][device["dev"]] = {}

        for group in device["groups"]:
            if "grouped_workqueues" in group and "grouped_engines" in group:
                if not group["dev"] in device_dict[device_gen][device["numa_node"]][device["dev"]]:
                    device_dict[device_gen][device["numa_node"]][device["dev"]][group["dev"]] = {'workqueues' : [], "engines" : []}
                for wqs in group["grouped_workqueues"]:
                    device_dict[device_gen][device["numa_node"]][device["dev"]][group["dev"]]["workqueues"].append(wqs["dev"])
                for engine in group["grouped_engines"]:
                    device_dict[device_gen][device["numa_node"]][device["dev"]][group["dev"]]["engines"].append(engine["dev"])

    return device_dict


def config_device(conf_file, dev_filter="", bof=False, is_root=False):
    print("Filter: " + dev_filter)

    if not os.path.exists(conf_file):
        raise ValueError(conf_file + " does not exist")

    ret, err, active_devices = accel_get_active_dvices()
    if len(active_devices):
        print("Disabling active devices")
        for device in active_devices:
            print("    " + device['dev'], end='')
            if device['dev'].find(dev_filter) != -1:
                ret, err, out = accel_disable_device(device['dev'], is_root=is_root)
                if ret:
                    print(" - error")
                else:
                    print(" - done")
            else:
                print(" - skipped")
    else:
        print("No active devices")

    print("Loading configuration", end='')
    ret, err, out = accel_load_config(conf_file, is_root=is_root)
    if ret:
        print(" - error")
        print("---------")
        print(err)
        print("---------")

    else:
        print(" - done")
 
    config_devices = open(conf_file, "r")
    config_devices = json.load(config_devices)
    print("Additional configuration steps")
    print("    Force block on fault: " + str(bof))
    for device in config_devices:
        if device['dev'].find(dev_filter) != -1:
            if device["groups"][0]["grouped_workqueues"]:
                for wq in device["groups"][0]["grouped_workqueues"]:
                    if bof:
                        ret, err, out = accel_set_block_on_fault(device["dev"], wq["dev"], bof_flag=True, is_root=is_root)
                        if ret:
                            print(" - error")
                            print("---------")
                            print(err)
                            print("---------")
    
    print("Enabling configured devices")
    for device in config_devices:
        print("    " + device["dev"], end='')
        if device['dev'].find(dev_filter) != -1:
            ret, err, out = accel_enable_device(device["dev"], is_root=is_root)
            if ret:
                print(" - error")
            else:
                print(" - done")

            if device["groups"][0]["grouped_workqueues"]:
                for wq in device["groups"][0]["grouped_workqueues"]:
                    print("        " + wq["dev"], end='')
                    ret, err, out = accel_enable_wq(device["dev"], wq["dev"], is_root=is_root)
                    if ret:
                        print(" - error")
                        print("---------")
                        print(err)
                        print("---------")
                    else:
                        print(" - done")

            else:
                print("        No work queues configured for the device")
        else:
            print(" - skipped")

    print("Checking configuration")
    ret, err, active_devices = accel_get_active_dvices()
    if len(active_devices):
        active_devices.sort(key=lambda x: x["dev"])
        for device in active_devices:
            for group in device["groups"]:
                if "grouped_workqueues" in group or "grouped_engines" in group:
                    print("    node: " + str(device['numa_node']) + "; device: " + device['dev'] + "; group: " + group["dev"])
                    if "grouped_workqueues" in group:
                        print("        wqs:     ", end='')
                        for wq in group["grouped_workqueues"]:
                            print(wq["dev"] + " ", end='')
                        print("")

                    if "grouped_engines" in group:
                        print("        engines: ", end='')
                        for engine in group["grouped_engines"]:
                            print(engine["dev"] + " ", end='')
                        print("")
    else:
        print("No active devices")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Accelerator configurator')
    parser.add_argument('--load', default='', metavar='FILE_NAME', help='Configuration file')
    parser.add_argument('--filter', default='', metavar='FILTER', help='Device filter')
    parser.add_argument('--bof', default=False, action='store_true', help='Set block on fault flag')
    parser.add_argument('--root', default=False, action='store_true', help='Use if sudo is required for device configuration')
    args = parser.parse_args()

    if args.load:
        config_device(args.load, args.filter, bof=args.bof, is_root=args.root)
