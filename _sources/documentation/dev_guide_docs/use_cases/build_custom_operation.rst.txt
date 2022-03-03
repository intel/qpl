 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Building Custom Operations
##########################


The Intel® Query Processing Library (Intel® QPL) library allows user to create 
a custom operation and then unite it with simple Intel QPL operations into the chain. 
Three ways of creating a custom operation are supported. Those are described next.


Inheritance
***********


The first option of creating a custom operation is inheritance from the
appropriate interface called ``custom_operation``.

Example:

.. code:: cpp

   class my_operation : public qpl::custom_operation
   {
   public:
       auto execute(const uint8_t *source,
                    uint32_t source_size,
                    uint8_t *destination,
                    uint32_t destination_size) -> uint32_t override
       {
           if (source_size > destination_size)
           {
               throw std::length_error("Not enough output length");
           }

           for (uint32_t i = 0; i < source_size; i++)
           {
               destination[i] = source[i];
           }

           return source_size;
       }
   };

   // ...
   auto custom_operation = my_operation();

   auto chain = qpl::scan_operation(qpl::equals, 7) |
                qpl::select_operation() |
                custom_operation |
                qpl::deflate_operation() |
                qpl::inflate_operation();

   qpl::execute(chain, source, destination);
   // ...


Lambdas
*******


The second option is using simple lambdas.

Example:

.. code:: cpp

   auto chain = qpl::scan_operation(qpl::equals, 7) |
                qpl::select_operation() |
                [](const uint8_t *source,
                   uint32_t source_size,
                   uint8_t *destination,
                   uint32_t destination_size) -> uint32_t
                {
                    if (source_size > destination_size)
                    {
                        throw std::length_error("Not enough output length");
                    }

                    for (uint32_t i = 0; i < source_size; i++)
                    {
                        destination[i] = source[i];
                    }

                    return source_size;
                } |
                qpl::deflate_operation() |
                qpl::inflate_operation();

   qpl::execute(chain, source, destination);

Note, that passed lambda is required to accept four parameters
(the same API as for previous option).


Functor
*******


The last option is to pass any callable object that satisfies the
requirements on passed arguments into the chain(they are listed in the
first option).

Example:

.. code:: cpp

   struct custom_functor
   {
       auto operator()(const uint8_t *source,
                       uint32_t source_size,
                       uint8_t *destination,
                       uint32_t destination_size) -> uint32_t
       {
           if (source_size > destination_size)
           {
               throw std::length_error("Not enough output length");
           }

           for (uint32_t i = 0; i < source_size; i++)
           {
               destination[i] = source[i];
           }

           return source_size;
       }
   };

   // ...
   auto chain = qpl::scan_operation(qpl::equals, 7) |
                qpl::select_operation() |
                custom_functor() |
                qpl::deflate_operation() |
                qpl::inflate_operation();

   qpl::execute(chain, source, destination);
   // ...
