# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

import argparse
import shutil
import os
import subprocess
import sys
import re
import csv
from bench_utils import *
import accel_conf

FIELDS_BASE = {CONFIG      : '',
               TIMING      : '',
               SRC_MEM     : '',
               DST_MEM     : '',
               API         : '',
               PATH        : '',
               EXEC        : '',
               OPERATION   : '',
               THREADS     : '',
               QDEPTH      : '',
               BATCH_SIZE  : '',
               DATA        : '',
               SIZE        : '',
               BLOCK_SIZE  : '',
               BLOCK       : '',
               COMP_PARAMS : '',
               FILT_PARAMS : '',
               RATIO       : ''}

FIELDS = {**FIELDS_BASE,
          THROUGHPUT  : '',
          LATENCY     : ''}

def run_generic(cmd, args, test):
    cmd = ["nice", "-n", "-20", "numactl", "--cpunodebind=0", "--membind=0", cmd] + args + ['--benchmark_out_format=csv', '--benchmark_counters_tabular=true', '--benchmark_min_time=0.3' if not test else '--benchmark_min_time=0']
    for arg in cmd:
        print(arg + " ", end='')
    print("")
    p = subprocess.run(cmd, universal_newlines=True)
    if p.returncode:
        print(cmd[0] + " - error")

def run_qpl(args, config_str, res_postfix, res_path):
    raw_res_path = args.res_path + "qpl_raw_" + config_str + res_postfix + ".csv"
    case_filter  = ""
    if args.filter:
        case_filter = "--benchmark_filter=" + args.filter
    run_generic(cmd=args.bin_path, args=['--dataset=' + args.data_path, '--benchmark_out=' + raw_res_path, case_filter], test=args.test)
    process_results(raw_res_path, res_path, config_str)

def process_results(raw_res_path, res_path, config_str):
    result_file = open(res_path, "a")
    result = csv.DictWriter(result_file, fieldnames=FIELDS.keys(), delimiter=';')
    if result_file.tell() == 0:
        result.writeheader()

    print("Processing report " + raw_res_path)
    raw_result_file = open(raw_res_path, "r")
    pos  = 0
    line = raw_result_file.readline()
    while line:
        if "name," in line:
            raw_result_file.seek(pos)
            break
        pos = raw_result_file.tell()
        line = raw_result_file.readline()
    raw_report = list(csv.DictReader(raw_result_file))
    raw_result_file.close()
    for line in raw_report:
        FIELDS.fromkeys(FIELDS, "")
        FIELDS[CONFIG]      = config_str
        FIELDS[OPERATION]   = find_pattern(line["name"], "^(.+?)/",             required=True)
        FIELDS[THREADS]     = find_pattern(line["name"], ".*/threads:(.+?)$",   required=False, empty=1)

        FIELDS[TIMING]      = find_param(line["name"], "timer",     required=True)
        FIELDS[SRC_MEM]     = find_param(line["name"], "in_mem",    required=True)
        FIELDS[DST_MEM]     = find_param(line["name"], "out_mem",   required=True)
        FIELDS[API]         = find_param(line["name"], "api",       required=True)
        FIELDS[PATH]        = find_param(line["name"], "path",      required=True)
        FIELDS[EXEC]        = find_param(line["name"], "exec",      required=True)
        FIELDS[QDEPTH]      = find_param(line["name"], "qsize",     required=True)
        FIELDS[BATCH_SIZE]  = find_param(line["name"], "batch")
        FIELDS[DATA]        = find_param(line["name"], "data")
        FIELDS[SIZE]        = find_param(line["name"], "size")
        FIELDS[BLOCK_SIZE]  = find_param(line["name"], "data:.+?/", delim="")
        FIELDS[BLOCK]       = find_param(line["name"], "block")
        FIELDS[COMP_PARAMS] = find_param(line["name"], "gen_path",  set=True) +\
                              find_param(line["name"], "huffman",   set=True) +\
                              find_param(line["name"], "stat_int",  set=True) +\
                              find_param(line["name"], "dict",      set=True) +\
                              find_param(line["name"], "lvl",       set=True, last=True)
        FIELDS[FILT_PARAMS] = "n/a"

        FIELDS[API] = "qpl_" + FIELDS[API]

        try:
            if line["Throughput"] == "" or line["Latency/Op"] == "" or line["Ratio"] == "":
                raise
            FIELDS[RATIO]       = float(line["Ratio"])
            FIELDS[THROUGHPUT]  = float(line["Throughput"])/1000000000 # GB/s
            FIELDS[LATENCY]     = float(line["Latency/Op"])*1000000000 # ns
        except:
            FIELDS[RATIO]       = "ERROR"
            FIELDS[THROUGHPUT]  = "ERROR"
            FIELDS[LATENCY]     = "ERROR"

        result.writerow(FIELDS)

    result_file.close()
    os.remove(raw_res_path)

def run_bench(args):
    config = {NUMAS:0, DEVICES:0, WQS:0, ENGINES:0}
    config[NUMAS], config[DEVICES], config[WQS], config[ENGINES] = accel_conf.get_aggregated(dev_filter="iax")
    config_str = str(config[NUMAS]) + "n" + str(config[DEVICES]) + "d" + str(config[ENGINES]) + "e" + str(config[WQS]) + "w"

    if args.clean and os.path.exists(args.res_path):
        shutil.rmtree(args.res_path)

    readers = []
    for rep in range(1, args.repetitions+1):
        res_postfix = ""
        if args.repetitions > 1:
            res_postfix = "_run_" + str(rep)
        print("Run " + str(rep))
        res_path = args.res_path + "qpl_" + config_str + res_postfix + ".csv"
        print("Results path: ", res_path)

        if not os.path.exists(args.res_path):
            os.makedirs(args.res_path)
        else:
            if os.path.exists(res_path):
                os.remove(res_path)

        run_qpl(args, config_str, res_postfix, res_path)

        if args.repetitions > 1:
            result_file = open(res_path, "r")
            readers.append(list(csv.DictReader(result_file, delimiter=';')));
            result_file.close()

    if args.repetitions > 1:
        res_path = args.res_path + "qpl_" + config_str + ".csv"
        result_file = open(res_path, "w+")
        result = csv.DictWriter(result_file, fieldnames=FIELDS.keys(), delimiter=';')
        result.writeheader()

        for row in range(len(readers[0])):
            min = dict(readers[0][row])
            for reader in range(1, args.repetitions):
                second = readers[reader][row]
                try:
                    if float(min[LATENCY]) > float(second[LATENCY]):
                        min[LATENCY]    = second[LATENCY]
                        min[THROUGHPUT] = second[THROUGHPUT]
                except:
                    if not isinstance(min[LATENCY], float):
                        min[LATENCY]    = second[LATENCY]
                        min[THROUGHPUT] = second[THROUGHPUT]
                    if isinstance(min[LATENCY], float) or isinstance(second[LATENCY], float):
                        print("Warning: unexpected error in run " + reader)
                        print(second)
            result.writerow(min)
        result_file.close()

    return res_path

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Benchmark runner for Intel® Query Processing Library (Intel® QPL)')
    parser.add_argument('--bin-path', default='./qpl_benchmarks', metavar='BIN_PATH', help='Path to benchmark')
    parser.add_argument('--res-path', default='./results/', metavar='RES_PATH', help='Path to results')
    parser.add_argument('--data-path', default='', metavar='DATA_PATH', help='Path to data set')
    parser.add_argument('--repetitions', default=1, type=int, metavar='REPETITIONS', help='Number of tests repetitions')
    parser.add_argument('--filter', default="", metavar='CASE', help='Filter for the benchmark')
    parser.add_argument('--clean', default=False, action='store_true', help='Clean results folder before measurements')
    parser.add_argument('--test', default=False, action='store_true', help='Test run with fastest options')
    args = parser.parse_args()

    print("Benchmark path: ", args.bin_path)

    run_bench(args)
