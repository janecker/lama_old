Examples
========

Log-Levels
----------

The first example shows a simple example with logging statements of all different levels.

.. literalinclude:: ../../logging/examples/LogLevels.cpp

At runtime it can be decided which logging will be printed:

.. code-block:: c++

    export SCAI_LOG=INFO
    examples/LogLevels 

    2015-08-19, 16:14:11 Demo @ main ( main -> LogLevels.cpp::44 ) INFO a message about progress in the program
    2015-08-19, 16:14:11 Demo @ main ( main -> LogLevels.cpp::47 ) WARN a message with a warning, but execution is still possible
    2015-08-19, 16:14:11 Demo @ main ( main -> LogLevels.cpp::48 ) ERROR a message for an error, error handling will be invoked
    2015-08-19, 16:14:11 Demo @ main ( main -> LogLevels.cpp::49 ) FATAL a message for a fatal error, execution will stop

.. code-block:: c++

    export SCAI_LOG=ERROR
    examples/LogLevels 

    2015-08-19, 16:14:25 Demo @ main ( main -> LogLevels.cpp::48 ) ERROR a message for an error, error handling will be invoked
    2015-08-19, 16:14:25 Demo @ main ( main -> LogLevels.cpp::49 ) FATAL a message for a fatal error, execution will stop

Using a config file (here with the name ``config``) is also possible, it should contain a line that sets the
level of the logger *Demo*.

.. code-block:: c++

    # config file for logging
    Demo = TRACE

.. code-block:: c++

    export SCAI_LOG=config
    2015-08-19, 16:43:25 Demo @ main ( main -> LogLevels.cpp::44 ) INFO a message about progress in the program
    2015-08-19, 16:43:25 Demo @ main ( main -> LogLevels.cpp::45 ) DEBUG a message useful to find bugs in the program
    2015-08-19, 16:43:25 Demo @ main ( main -> LogLevels.cpp::46 ) TRACE a message with very detailled info, usually not compiled
    2015-08-19, 16:43:25 Demo @ main ( main -> LogLevels.cpp::47 ) WARN a message with a warning, but execution is still possible
    2015-08-19, 16:43:25 Demo @ main ( main -> LogLevels.cpp::48 ) ERROR a message for an error, error handling will be invoked
    2015-08-19, 16:43:25 Demo @ main ( main -> LogLevels.cpp::49 ) FATAL a message for a fatal error, execution will stop

Multiple-Thread-Environment
---------------------------

 The next example demonstrates the use of logging with multiple threads.

.. literalinclude:: ../../logging/examples/LogOpenMP.cpp

.. code-block:: c++

   export SCAI_LOG=INFO
   examples/LogOpenMP

   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_4 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 8 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_2 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 4 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_4 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 9 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_1 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 2 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_3 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 6 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_2 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 5 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_5 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 10 of 20
   2015-08-19, 16:11:01 LogOpenMP @ main ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 0 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_7 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 14 of 20
   2015-08-19, 16:11:01 LogOpenMP @ main ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 1 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_7 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 15 of 20
   2015-08-19, 16:11:01 LogOpenMP @ main ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 16 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_1 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 3 of 20
   2015-08-19, 16:11:01 LogOpenMP @ main ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 17 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_1 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 18 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_3 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 7 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_1 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 19 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_6 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 12 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_5 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 11 of 20
   2015-08-19, 16:11:01 LogOpenMP @ OMP_Thread_6 ( main -> LogOpenMP.cpp::29 ) INFO executes iteration 13 of 20
    