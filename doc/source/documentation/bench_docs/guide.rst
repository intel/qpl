 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _library_benchmarking_reference_link:

Benchmarks Framework Guide
##########################

Goal of the benchmarks is to provide performance guidance on various use case scenarios
of the Intel® Query Processing Library (Intel® QPL).
To cover these cases the benchmarks provide several performance metrics and modes of operation.

.. attention::

    Currently, the Intel QPL benchmarks framework offers limited support.

    - Compression and decompression are supported for fixed and dynamic modes; canned mode is not supported.
    - Huffman only mode is not supported.
    - Analytic operations are not supported.

Quick Start
***********

Benchmark are based on the Google benchmark library and built as a part of Intel QPL.
Refer to :ref:`Installation page <building_library_build_reference_link>`
for details on how to build the library and resolve all prerequisites.

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

    sudo ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="deflate.*:iaa.*:sync.*:fixed.*" --benchmark_min_time=0.1 --block_size=0

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

Key Terms
*********

* **API**: Currently, only Low-Level C API is supported. Case naming: ``api:c``.
* **Path**: Represents execution path for the library. Case naming: ``path:iaa`` for executing on accelerator, ``path:cpu`` for running on CPU.
* **Execution mode**: defines synchronous or asynchronous execution. Case naming: ``exec:async``, ``exec:sync``.

    **Sync mode**: each measurement loop one call to Intel QPL operation is submitted always followed by blocking wait.
    Thus only one operation is processed at a time and only one engine of accelerator is loaded.
    Is not affected by ``--threads`` argument. Main output of sync mode is **Latency** metric.
    ``--queue_size`` can be used to submit several operations at once to measure latency of the various queue sizes.

    **Async mode**: each case spawns ``--threads`` number of threads and each thread runs it's own measurement loop.
    For each loop benchmark will submit ``--queue_size``/``--threads`` operations without blocking wait
    and resubmit operation as soon as it is completed always keeping device busy.
    Main output of async mode is **Throughput** metric.
    For small workloads higher number of threads may be required to saturate devices,
    for big workloads even one thread may reach capacity.

* **Blocks**: Input data is split by blocks of size ``--block_size=XXXX`` (with XXXX being in bytes) and each block is processed separately.
* **Dataset**: Path should be provided to a dataset (e.g., Calgary corpus) via ``--dataset=<dataset_dir>`` parameter.

Using the Benchmarks Framework
******************************

Intel QPL benchmark framework is based on Google benchmark library,
run ``./qpl_benchmarks --help`` to get a full list of supported commands and input arguments with detailed description.

.. attention::

    If no accelerators are available on the system, you can use ``--no_hw`` to suppress Intel IAA initialization check warning.

In order to set up a specific run configuration ``--benchmark_filter`` should be used,
which input is a regexp based on the case name.

For instance, for launching compression operation using Low-Level C API, synchronous execution and fixed mode
on a CPU, use the next expression ``--benchmark_filter="deflate.*:c/.*:cpu.*:sync.*fixed"``.

Executing on Hardware Path
==========================

.. warning::

    Make sure to resolve :ref:`requirements for running on hardware path <system_requirements_hw_path_reference_link>` and
    :ref:`configure accelerator <accelerator_configuration_reference_link>` before executing the example.

.. attention::

    Currently, the library doesn't provide a way to specify a number of Intel IAA
    instance for execution and will use everything available on the system.

    It is the user's responsibility to configure the accelerator.
    If you need to run on 1 or multiple Intel IAA instances, make sure your system is configured appropriately.

.. attention::

    By default, Intel QPL would use Intel IAA instances located on the NUMA node of the calling process.

    If you need to specify NUMA node for execution,
    use appropriate NUMA policy (for instance, ``numactl --cpunodebind <numa_id> --membind <numa_id>``)
    or add ``--node=<numa_id>`` to execution command.

    It is user responsibility to configure accelerator and ensure device(s) availability on the NUMA node.
    Refer to :ref:`library_numa_support_reference_link` section for more details.

Latency Tests
=============

For reporting or tracking latency metric, use ``sync`` mode, 1 Intel IAA instance, and a single thread.

Below are examples for compression (``deflate``) and decompression (``inflate``) using 4kb block_sizes:

.. code-block:: shell

    numactl --membind=0 --cpunodebind=0 sudo ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="deflate.*:iaa.*:sync.*:fixed.*" --benchmark_min_time=0.1 --block_size=4096


.. code-block:: shell

    numactl --membind=0 --cpunodebind=0 sudo ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="inflate.*:iaa.*:sync.*:fixed.*lvl:1.*" --benchmark_min_time=0.1 --block_size=4096

Throughput Tests
================

For reporting or tracking throughput metric, use ``async`` mode, 1 to 4 Intel IAA devices, and multiple threads.

Below are examples for compression (``deflate``) and decompression (``inflate``) using 4kb block_size and ``queue_size=128``:

.. note::

    ``--threads=2`` might not be enough for inflate operation to saturate full capacity,
    so it is recommended to use ``--threads=6`` or ``--threads=8`` to get best and stable results.

.. code-block:: shell

    numactl --membind=0 --cpunodebind=0 sudo ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="deflate.*:c/.*:iaa.*:async.*:fixed.*" --benchmark_min_time=0.5 --block_size=4096 --queue_size=128 --threads=2


.. code-block:: shell

    numactl --membind=0 --cpunodebind=0 sudo ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="inflate.*:c/.*:iaa.*:async.*:fixed.*lvl:1.*" --benchmark_min_time=0.5 --block_size=4096 --queue_size=128 --threads=8




