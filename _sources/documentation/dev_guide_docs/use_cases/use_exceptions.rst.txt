 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _using_exceptions_reference_link:

Using Exceptions
################


The main method inherent in all exceptions is ``char* what()``, which
allows you to view an error message.

You can catch exceptions of a specific type like
``qpl::invalid_argument_exception``, as well as all exceptions
inherited from the specified type like
``qpl::invalid_argument_exception &``.


Exception Handling Examples
***************************


-  only exceptions of specific type:

.. code:: cpp

   #include <qplhl/util/exceptions.hpp>

   int main()
   {
       try
       {
           // Some code that may throw an invalid_argument_exception
       }
       catch(qpl::invalid_argument_exception e)
       {
           std::cerr << e.what() << std::endl;
       }
   }


-  the whole exceptions tree of a selected type:

.. code:: cpp

   #include <qplhl/util/exceptions.hpp>

   int main()
   {
       try
       {
           // Some code that may throw an invalid_argument_exception or its descendants
       }
       catch(qpl::invalid_argument_exception &e)
       {
           std::cerr << e.what() << std::endl;
       }
   }


Exception Throw Example
***********************


.. code:: cpp

   #include <qplhl/util/exceptions.hpp>

   int main()
   {
       try
       {
           ...
           throw invalid_data_exception("Invalid Huffman table");
       }
       catch(qpl::operation_process_exception &e)
       {
           std::cerr << e.what() << std::endl;
       }
   }
