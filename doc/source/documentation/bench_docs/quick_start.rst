 .. ***************************************************************************
 .. * Copyright (C) 2024 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _library_benchmarking_quick_start_link:

Quick Start
###########

In order to start using Benchmarks Framework, resolve all prerequisites and build the library,.
To do so, refer to the :ref:`Installation page <building_library_build_reference_link>`.

The example below demonstrates running Deflate using Fixed block on accelerator using synchronous execution.

.. warning::

    Make sure to resolve :ref:`requirements for running on hardware path <system_requirements_hw_path_reference_link>` and
    configure :ref:`Intel® In-Memory Analytics Accelerator (Intel® IAA) <accelerator_configuration_reference_link>`
    before executing the example.

.. attention::

    By default Benchmarks do not set :c:member:`qpl_job.numa_id` value, so the library will auto detect NUMA node
    of the calling process and use Intel® In-Memory Analytics Accelerator (Intel® IAA) device(s) located on the same node.

    If you need to specify NUMA node for execution of this example,
    use appropriate NUMA policy (for instance, ``numactl --cpunodebind <numa_id> --membind <numa_id>``)
    or add ``--node=<numa_id>`` to execution command.

    It is user responsibility to configure accelerator and ensure device(s) availability on the NUMA node.

    Refer to :ref:`library_numa_support_reference_link` section for more details.

.. code-block:: shell

    ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="deflate.*:iaa.*:sync.*:fixed.*" --benchmark_min_time=0.1s --block_size=0

.. code-block:: shell
    :caption: Output to terminal:

    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Benchmark                                                                                                                          Time             CPU   Iterations UserCounters...
    ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    deflate/api:c/path:iaa/exec:sync/qsize:1/in_mem:llc/out_mem:cc_ram/timer:proc/data:pic.113/huffman:fixed/lvl:1/real_time        3226 ns         3226 ns        42981 Latency=3.22604us Latency/Op=3.22604us Ratio=4.40904 Throughput=1.26967G/s
    deflate/api:c/path:iaa/exec:sync/qsize:1/in_mem:llc/out_mem:cc_ram/timer:proc/data:pic.28/huffman:fixed/lvl:1/real_time         9304 ns         9304 ns        14907 Latency=9.30406us Latency/Op=9.30406us Ratio=5.06461 Throughput=1.76095G/s
    deflate/api:c/path:iaa/exec:sync/qsize:1/in_mem:llc/out_mem:cc_ram/timer:proc/data:progp.02/huffman:fixed/lvl:1/real_time       9454 ns         9454 ns        14828 Latency=9.45413us Latency/Op=9.45413us Ratio=2.88756 Throughput=1.733G/s
    deflate/api:c/path:iaa/exec:sync/qsize:1/in_mem:llc/out_mem:cc_ram/timer:proc/data:progp.08/huffman:fixed/lvl:1/real_time       3255 ns         3255 ns        43100 Latency=3.25501us Latency/Op=3.25501us Ratio=3.25338 Throughput=1.25837G/s
    deflate/api:c/path:iaa/exec:sync/qsize:1/in_mem:llc/out_mem:cc_ram/timer:proc/data:trans.04/huffman:fixed/lvl:1/real_time       9436 ns         9436 ns        14782 Latency=9.43575us Latency/Op=9.43575us Ratio=2.41759 Throughput=1.73637G/s
    deflate/api:c/path:iaa/exec:sync/qsize:1/in_mem:llc/out_mem:cc_ram/timer:proc/data:trans.21/huffman:fixed/lvl:1/real_time       3243 ns         3243 ns        43891 Latency=3.24297us Latency/Op=3.24297us Ratio=3.86051 Throughput=1.26304G/s
