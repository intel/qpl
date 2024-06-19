 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Asynchronous Execution
######################

The function :c:func:`qpl_execute_job` in :ref:`quick_start_reference_link` is the
*synchronous* interface. It does not return until the job has completed. If the application
wants to do other work while the job is being processed on the accelerator, it can use the
*asynchronous* interface. In this case, the application specifies the ``Hardware Path`` or ``Auto Path``
for the execution and submits the job using :c:func:`qpl_submit_job`. The status of the job can then be
periodically queried by :c:func:`qpl_check_job`. The application may call :c:func:`qpl_wait_job`
to wait until the job completes.

.. note::
    Synchronous interface :c:func:`qpl_execute_job` is essentially a
    combination of the asynchronous interface :c:func:`qpl_submit_job` followed by
    :c:func:`qpl_wait_job`.

.. _library_async_with_auto_reference_link:

Asynchronous Execution and ``Auto Path``
****************************************

When the application specifies ``Auto Path`` for an asynchronous execution, host fallback may happen inside
:c:func:`qpl_submit_job`, :c:func:`qpl_wait_job`, or :c:func:`qpl_check_job`.

If host fallback doesn't happen, these APIs will return status from the asynchronous accelerator execution.

If host fallback happens inside an API, it will switch to execution on the CPU host, finish the execution, and return its status.
After the host fallback has happened, subsequent calls to :c:func:`qpl_wait_job` and :c:func:`qpl_check_job`
will return :c:macro:`QPL_STS_OK` to indicate that the execution has already completed.