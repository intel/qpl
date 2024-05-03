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

.. warning::
   Currently, ``Auto Path`` with asynchronous execution is not supported.

Note that the synchronous interface :c:func:`qpl_execute_job` is essentially a
combination of the asynchronous interface :c:func:`qpl_submit_job` followed by
:c:func:`qpl_wait_job`.
