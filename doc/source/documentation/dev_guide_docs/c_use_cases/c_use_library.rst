 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Library Usage Scenarios
#######################


The basic idea is that the application fills in a “job” data structure
with the necessary parameters to describe the job to be done. This is
then passed to the library, and when it completes, the job structure
contains the results (error codes, output parameters, etc.).

The Intel® Query Processing Library (Intel® QPL) does not perform any 
internal memory allocations. All memory buffers that are required for 
the library work must be provided by an application. As the library supports several
implementation/execution paths (*qpl_path_hardware*, *qpl_path_software*,
and *qpl_path_auto*), the size of the required memory buffer for job
structure is not predefined and depends on the used CPU/HW and execution
path. Therefore, before submitting any job to the library, the
application must determine the size of the required memory buffer,
allocate this memory buffer, and perform initialization of this memory
buffer. The algorithm of these steps is as follows:


1. Call the ``qpl_get_job_size()`` function:

.. code-block:: c

    status = qpl_get_job_size(qpl_path, job_size_ptr);

where:

- ``status`` - has type ``qpl_status`` and contains execution status of the function - success 
  or error code (see ``qpl.h`` for more details).
- ``qpl_path`` - has type ``qpl_path_t`` that can be:
  
  - ``qpl_path_hardware`` - all hardware-supported functions are executed by the 
    Intel® In-Memory Analytics Accelerator (Intel® IAA);
  - ``qpl_path_software`` - all supported functionality is executed by the software library;
  - ``qpl_path_auto`` - the library automatically dispatches execution of the requested jobs 
    either to the Intel IAA or to the software library depending on internal heuristics;

- ``job_size_ptr`` - has type ``uint32_t*`` (a pointer to unsigned int) and stores 
  the calculated memory buffer size returned by the function.


2. Allocate the required memory buffer (size in bytes = ``*job_size_ptr``).
   The library does not allocate or free memory, therefore an application must 
   provide required memory buffer for the job structure and free it after it becomes
   unnecessary. During allocation, the memory buffer should be casted to
   ``qpl_job*`` data type, for example:

.. code-block::
  
  qpl_job_ptr = (qpl_job\*)malloc(\*job_size_ptr);


**Check allocation results for success.**


3. Call the ``qpl_init_job()`` function:

.. code-block:: c

    status = qpl_init_job(qpl_path, qpl_job_ptr);


Once the ``qpl_job`` initialized, it does not require reinitialization
and updating of output fields before reusing.

where:

- ``status`` - has type ``qpl_status`` and contains execution status 
  of the function - success or error code (see ``qpl.h`` for more details).
- ``qpl_path`` - has type ``qpl_path_t`` that can be:
  
  - ``qpl_path_hardware`` - all hardware-supported functions are executed by the Intel IAA;
  - ``qpl_path_software`` - all supported functionality is executed by the software library;
  - ``qpl_path_auto`` - the library automatically dispatches execution of the requested jobs 
    either to the Intel IAA or to the software library dependending on internal heuristics;
    Note: It shall have the same value as in the 1st step (the call to ``qpl_get_job_size()``).

- ``qpl_job_ptr`` - has type ``qpl_job`` (job structure) and stores the initialized job 
  structure returned by the function.


An example of C-code for these first steps:

.. code-block:: c

  uint32_t size; 
  qpl_job *qpl_job_ptr;
  qpl_status status;

  status = qpl_get_job_size(qpl_path_auto, &size);
  qpl_job_ptr = (qpl_job*)malloc(size);
  status = qpl_init_job(qpl_path_auto, qpl_job_ptr);


After the application initializes the job structure, it can request any
library operation needed.

The simplest interface is ``qpl_execute_job(job)``. It does not return any
result until the job completes. This is the *synchronous* interface. The
*asynchronous* interface is available as well. In this case, the job is
submitted using ``qpl_submit_job(job)``. The application can then
periodically query the status of the job with ``qpl_check_job(job)``. The
application may call ``qpl_wait_job(job)`` to wait until the job completes.

The ``qpl_execute_job()`` function is essentially a
combination of ``qpl_submit_job()`` followed by ``qpl_wait_job()``.

In the context of the behavioral model (i.e. without actual hardware
support), the job is processed when submitted. The ``qpl_wait_job()``
function always returns completed status. This is not the case when real
hardware is involved.

In some cases, for example, *compression* and *decompression*, a larger
overall task may be broken into a series of separate library calls. For
example, an application compressing a large file might call the library
repeatedly with 64kB buffers, until the end of the file is reached.
These calls are related to each other as they use the same job
structure. Certain flags indicate whether a particular job submission 
is the start of a new overall task or it is a continuation of the previous one. 
Because of this, there is not a separate “init” function; the initialization is 
implied by the job's parameters.

For more information on Intel QPL job structure description,
refer to the :ref:`Job Structure <c_job_structure_reference_link>` chapter.
