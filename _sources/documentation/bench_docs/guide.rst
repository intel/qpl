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
Benchmark are based on the Google benchmark library and built as a part of Intel QPL.

.. attention::

    Currently, the Intel QPL benchmarks framework offers limited support.

    - Compression and decompression are supported for Fixed, Dynamic and Canned modes.
    - CRC operations are supported. The supported CRCs are the default CRC64, CRC32 (Gzip), CRC32 (wimax),
      CRC32-C (ICSCI), CRC-16-T10-DIF, and CRC-16-CCITT.
    - Huffman only mode is not supported.
    - Analytic operations are not supported.

To learn how to start using Benchmarks and run simple operation, refer to the :ref:`Quick Start page <library_benchmarking_quick_start_link>`.
For more detailed information on the Benchmarks Framework, refer to the next sections.

Using the Benchmarks Framework
******************************

To get a full list of supported commands and input arguments with detailed description, run ``./qpl_benchmarks --help``.
Intel QPL Benchmark Framework is based on Google benchmark library and supports all its command line arguments
as well as additional ones that are specific to Intel QPL.

.. attention::

    By default, Benchmarks Framework would attempt to do an initialization check for the accelerator.
    If no accelerators are available on the system, you can use ``--no_hw`` to suppress
    Intel® In-Memory Analytics Accelerator (Intel® IAA) initialization check warning.

Key Terms
=========

Below is the list of key terms used in the Benchmarks Framework.
Some of them are input parameters and some are used in the case naming.
For the latter, refer to the next section for more details on how to set up the correct filter.

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

* **Compression mode**: Currently, Static mode is not supported. Supported case namings: ``huffman:fixed``, ``huffman:dynamic`` or ``huffman:canned``. Note, that for Canned mode, Huffman tables are precomputed and do not contribute to the metrics measured.
* **Blocks**: Input data is split by blocks of size ``--block_size=XXXX`` (with XXXX being in bytes) and each block is processed separately.
* **Dataset**: Path should be provided to a dataset (e.g., Calgary corpus) via ``--dataset=<dataset_dir>`` parameter.

Setting the Correct Filter for Performance Measurement
======================================================

In order to set up a specific run configuration ``--benchmark_filter`` should be used,
which input is a regexp based on the case name.

For instance, for launching compression operation using Low-Level C API, synchronous execution and fixed mode
on a CPU, use the next expression ``--benchmark_filter="deflate.*:c/.*:cpu.*:sync.*fixed"``.
To run decompression benchmarks, run the filter with ``inflate``.

To run CRC benchmarks, run the filter with ``crc``. To filter for a specific CRC operation, run the filter with
one of the following phrases at the end: ``crc32_gzip``, ``crc32_iscsi``, ``crc32_wimax``, ``T10DIF``, ``crc16_ccitt``,
``crc64``. For example, to run CRC benchmarks on only crc64,
the following filter would work: ``--benchmark_filter="crc.*:c/.*:cpu.*:sync.*crc64"``.

Executing using Accelerators
============================

.. attention::

    It is the user's responsibility to configure the accelerator and ensure the availability of the device(s).

    Make sure to resolve :ref:`requirements for running on hardware path <system_requirements_hw_path_reference_link>`
    and :ref:`configure accelerator <accelerator_configuration_reference_link>` before executing using accelerator.

Benchmark Framework does not support choosing a specific Intel IAA instance for execution.
However, it is possible to limit execution to devices only from a certain NUMA node or socket using the ``--node=<integer>`` option.

.. note::
    By default, when ``--node=<integer>`` is unset, :c:member:`qpl_job.numa_id` value is set to ``-1`` for Benchmarks execution.

    If the Intel QPL version is **`< 1.6.0`**, the library will auto-detect the NUMA node of the calling process
    and use the Intel® In-Memory Analytics Accelerator (Intel® IAA) device(s) located on the same **NUMA node**.

    If the Intel QPL version is **`>= 1.6.0`**, the library will use the Intel IAA device(s) located on the **socket** of the calling thread.

All possible values for ``--node=<integer>`` are as follows:

- Any value ``>= 0`` is considered as a valid NUMA node ID, and only the Intel IAA device(s) located on the specified NUMA node would be used.
- ``-1`` (:c:macro:`QPL_DEVICE_NUMA_ID_SOCKET`) is a **default** and used for selecting the Intel IAA device(s) located on the same socket as the calling thread.
- ``-2`` (:c:macro:`QPL_DEVICE_NUMA_ID_CURRENT`) could be used for selecting the Intel IAA device(s) located on the same NUMA node as the calling thread.
- ``-3`` (:c:macro:`QPL_DEVICE_NUMA_ID_ANY`) could be used for selecting any available Intel IAA device(s).

Latency Tests
=============

For reporting or tracking latency metric, it is recommended to use ``sync`` mode, 1 Intel IAA instance, and a single thread.

Below are examples for compression (``deflate``) and decompression (``inflate``) using 4kb block_sizes and Fixed mode:

.. code-block:: shell

    ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="deflate.*:iaa.*:sync.*:fixed.*" --benchmark_min_time=0.1s --block_size=4096


.. code-block:: shell

    ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="inflate.*:iaa.*:sync.*:fixed.*lvl:1.*" --benchmark_min_time=0.1s --block_size=4096

Throughput Tests
================

For reporting or tracking throughput metric, use ``async`` mode, 1 to 4 Intel IAA devices, and multiple threads.

Below are examples for compression (``deflate``) and decompression (``inflate``) using 4kb block_size, ``queue_size=128``
and Fixed mode:

.. note::

    ``--threads=2`` might not be enough for inflate operation to saturate full capacity,
    so it is recommended to use ``--threads=6`` or ``--threads=8`` to get best and stable results.

.. code-block:: shell

    ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="deflate.*:c/.*:iaa.*:async.*:fixed.*" --benchmark_min_time=0.5s --block_size=4096 --queue_size=128 --threads=2


.. code-block:: shell

    ./<install_dir>/bin/qpl_benchmarks --dataset=<dataset_dir>/ --benchmark_filter="inflate.*:c/.*:iaa.*:async.*:fixed.*lvl:1.*" --benchmark_min_time=0.5s --block_size=4096 --queue_size=128 --threads=8

.. toctree::
   :maxdepth: 1
   :hidden:

   quick_start.rst
