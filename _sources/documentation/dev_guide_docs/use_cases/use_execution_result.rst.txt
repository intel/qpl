 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _using_execution_result_reference_link:


Using execution_result
######################


Class contains three methods of handling a result of execution:

1) :ref:`Common handling <common_handling_link>` method:
   
   .. code-block:: cpp

       template<class present_statement_t, class absent_statement_t>
       void handle(present_statement_t present_statement, absent_statement_t absent_statement) const;

2) :ref:`Present handling <present_handling_link>` method:
   
   .. code-block:: cpp

       template<class present_statement_t>
       void if_present(present_statement_t present_statement) const;

3) :ref:`Absent handling <absent_handling_link>` method:
   
   .. code-block:: cpp

       template<class absent_statement_t>
       void if_absent(absent_statement_t absent_statement) const;


where 

- ``present_statement`` - lambda function with action
  to be performed when the status is zero.
- ``absent_statement`` - lambda function with action to be
  performed when the status is not zero.


.. _common_handling_link:

Common Handling
***************


Method that performs the given actions depending on status:

.. code:: cpp

   ...
   result.handle([](uint32_t value) -> void
                 {
                     std::cout << "Result size: " << value << std::endl;
                 }, [](uint32_t status) -> void
                 {
                     throw std::runtime_error("Error: Status code - " + std::to_string(status));
                 });
   ...


.. _present_handling_link:

Present Handling
****************


Method that performs the given action if the result was present:

.. code:: cpp

   ...
   result.if_present([](uint32_t value) -> void
                    {
                        std::cout << "Result size: " << value << std::endl;
                    });
   ...


.. _absent_handling_link:

Absent Handling
***************


Method that performs the given action if the result was absent:

.. code:: cpp

   ...
   result.if_absent([](uint32_t value) -> void
                   {
                       throw std::runtime_error("Error: Status code - " + std::to_string(status));
                   });
   ...
