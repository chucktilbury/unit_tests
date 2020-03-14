# Unit Testing

This document outlines how to use the unit testing framework implemented by this code. This is a very simple testing framework that does not catch signals or exceptions. It is intended solely for the purpose of testing individual functions to verify their behavior. It is not the intention of this framework to verify that different units interact properly. 

I built this for my own use on other projects. If you want to make suggestions or report a bug, please do so by all means through github where this source code is hosted.

To quick start this, download it and simply run make. It should all build and run if you are using a fairly modern C compiler. There should be no system dependencies and very few dependencies at all. This framework does use the standard library and I consider that a deficiency. However it works well enough for what I need and I am currently using it with toi2. That is the Toy Object Interpreter. It's a pet project that keeps me out the bars at night. 

A test suite is a stand-alone program that consists of exactly one file. They should rarely need to link with other modules. To do so defeats isolating the code so that it can be tested in the simplest way that is practical. Code that needs to be linked with a lot of other modules cannot be considered "testable" using this framework.

The testing header should be included as the first include in all of the tests. The header contains all of the code that is common to all of the testing, such as functions to print results and identify tests. It has definitions and includes that are required for the testing infrastructure. Each test file is a stand-alone program that has a minimum of external dependencies. An individual test file should not need many other includes other than the ones supplied in the unit_tests header, except for includes that contain mocks and stubs.

The mocks should have an extension of *.mock and likewise stubs should conform to *.stub. They could be anything but this is how the demonstration code is written. Mocks and stubs can also be embedded directly in the test suite file. This is the normal case.

A mock is defined as a function that returns a specific value that is used in the test to verify a code path that may or may not involve an error. In general, any function that is called inside a unit under test can be mocked. 

A stub is defined as a function that does not actually return a value. A stub is generally needed to make the test link properly. In practice, stubs and mocks can be used interchangeably.

System calls, such as malloc() and friends can be mocked with special functions that allow memory usage to be tracked and reported at the end of the test, or they can be mocked with a "regular" mock. It should be very easy to extend this functionality using the existing code as a template. If the built in mocks are used, then USE_MEMORY must be enabled in the test. See below for details.

An example test:
```C
/*
 * These tests verify that the FIFO functionality fails "properly" when
 * calloc() returns NULL.
 */
#define USE_MEMORY 0
#define VERBOSE 3

#include "unit_tests.h"

// actual prototype: void fatal_error(char *str, ...);
DEF_STUB(void, fatal_error, str, ...) 
    (void)str;
END_STUB

DEF_MOCK(void*, malloc, size_t size)
    (void)size;
    return NULL;
END_MOCK
    
#include "../../../fifo.c" /* module under test */

DEF_TEST(create_fifo_fails)
    void *val = create_fifo();
    assert_ptr_is_null(val);
	assert_mock_entered("malloc");
    assert_stub_entered("fatal_error");
END_TEST

DEF_TEST(fifo_invalid_ptr_fails) 
    fifo_add(NULL, NULL, 0);
    assert_stub_entered("fatal_error");
END_TEST

DEF_TEST_MAIN
    TRACK_MOCK("malloc");
    TRACK_STUB("fatal_error");
    ADD_TEST(fifo_invalid_ptr_fails);
    ADD_TEST(create_fifo_fails);
END_TEST_MAIN
```

See the example test suites for more detail. 

## Configuration Parameters

The configuration parameters are used to add or remove functionality from the test harness. They are declared as macros before the unit_tests.h is included. 

 * USE_MEMORY

    This enables the builtin mocks for malloc(), calloc(), realloc(), free(), and strdup(). If memory is enabled, then the test tracks how much memory has been allocated by the functions under test and you can do asserts based on that information. See below for more details.

     * 0 = do not use any memory allocation tracking

     * 1 = use memory allocation tracking

        

 * USE_CAPTURE
 
Capture allows a function call within a function under test to appear to abort the program, such as when a function calls exit(). When it is placed within a capture block, and used with a mock, the execution of the test can continue without exiting the program. If capture is enabled then every instance of a function that calls the mock **must** be placed in a capture block. See example tests for more information.

 * 0 = No test needs to use capture.
  
  * 1 = Capture is being used by a test.
 
      

 * VERBOSE

     This parameter controls how much text is output for each test. Note that errors are always printed.

      *  0 = print summary only
      *  1 = print failures only
      *  2 = print pass and fail messages
      *  3 = start and end messages
      *  4 = or greater enables debugging messages at that level.
      

 * MAX_MOCKS

     Maximum number of mocks that can be tracked. (default is 10)

     

*  MAX_STUBS

     Maximum number of stubs that can be defined. (default is 10)

     

 *  MAX_TESTS
 
Maximum number of tests that can be defined (default is 20)

## Defining Mocks and Stubs

There can be any number or combination of mocks and stubs. They can contain any code that a normal C function can contain, including macros and comments. For example, if you want to mock a function that has a prototype that looks like:

```C
void *some_func(int num, const char* str);
```

The mock (or stub) definition looks like this:

```c
DEF_MOCK(void*, some_func, int num, const char *str)
    (void)num;
	(void)str;
	return NULL;
END_MOCK
```

* Note that the unused function arguments have been "voided" to avoid a compiler warning.  
* Instead of the normal syntax, the elements of the function definition are listed in a tuple so that they can be individually manipulated by the macros.
* There are no curly braces around a mock (or stub) definition. Those are supplied by the macros for visual clarity.
* Stub definitions use DEF_STUB ... END_STUB, instead of the mock definition.

### Assertions for Mocks and Stubs

* assert_mock_entered_count(v, n) 

* assert_stub_entered_count(v, n) 

  Assert on how many times a mock or stub has been entered. If the number of times the mock has been entered matches the count then the assert passes.

  * v = the expected number

  * n = then name of the mock as a quoted string

    

* assert_mock_entered(n) 

* assert_stub_entered(n) 

  Assert if the mock or stub has not been entered. The test passes if the mock has been entered at least once.

  * n = The name of the mock as a quoted string.

    

* assert_mock_not_entered(n) 

* assert_stub_not_entered(n) 

  Fail if the mock or stub has not been entered. 

  * n = The name of the mock as a quoted string.

## Generic Assert Macros Defined

These assert macros are fairly generic and used in general testing. 

* assert_int_equal(e, g) 

* assert_uint_equal(e, g) 

* assert_string_equal(e, g) 

* assert_string_not_equal(e, g) 

* assert_int_not_equal(e, g) 

* assert_uint_not_equal(e, g) 

  These macros test for equality on simple values. For example, ```assert_int_equal()``` passes if the values presented are equal.

  * e = The expected value.
  * g = The value produced by the test.

* assert_double_equal(e, g, p) 

* assert_double_not_equal(e, g, p) 

  The macros that test a floating point number for equality need to test for a range of values. The extra parameter (precision) specifies that range. So, if you are checking the two numbers 10.01 and 10.1 for equality, if the precision is set to 0.01, the test (for equality) fails. If the precision is set to 0.10, then the test passes.

  * e = The expected value.
  * g = The value generated by the test.
  * p = precision

* assert_ptr_null(p) 

* assert_ptr_not_null(p) 

  These macros assert on whether a pointer is NULL or not. Very simple.

  * p = The pointer to check.

* assert_buffer_equal(p1, p2, s) 

* assert_buffer_not_equal(p1, p2, s) 

  These macros compare two buffers for equality. They are compared byte by byte up to the number of bytes specified by the parameter (s). If every byte in both buffers match, then the assert passes.

  * p1 = Pointer to the first buffer.
  * p2 = Pointer to the second buffer.
  * s = The number of bytes to examine.

## Memory Pool Mocks

When USE_MEMORY is on, the memory pool macros and functions are added to the test. These functions behave as mocks for some of the memory allocation routines in the standard library. Specifically, malloc(), calloc(), realloc(), free() and strdup() are mocked. These mocks are simple wrappers around the standard library routines that have some added instrumentation to track how much memory has been allocated and how much is currently allocated. It is also possible to tell how many times these functions have been called but a function under test. When these mocks are turned on, they are valid for the whole test file. There is no way to enable them for a single test in a file. Also, you cannot define a mock using DEF_MOCK called "malloc" if USE_MEMORY is turned on because you will get a linker error.

### Memory Pool Assertions

* assert_memory_pool_size(n) 

  This assert checks for the number of bytes that are currently allocated in the test. This value is reset for each test. Calling free() reduces the size of the pool and calling malloc() increases it. 

  * n = The expected size of the pool.

* assert_memory_pool_not_zero() 

  Checks to see if the memory pool is not zero. If it is, then the assertion fails.

* assert_memory_pool_zero() 

  Checks to see if the memory pool is zero. If it is, then the assert passes.

* assert_malloc_entered() 

* assert_malloc_not_entered() 

* assert_malloc_entered_count(v) 

  These macros check to see if malloc() has or has not been entered.

  * v = The number of times malloc() has been entered.

* assert_calloc_entered() 

* assert_calloc_not_entered() 

* assert_calloc_entered_count(v) 

  These macros check to see if calloc() has or has not been entered.

  * v = The number of times malloc() has been entered.

* assert_free_entered() 

* assert_free_not_entered() 

* assert_free_entered_count(v) 

  These macros check to see if free() has or has not been entered.

  * v = The number of times free() has been entered.

* assert_realloc_entered() 

* assert_realloc_not_entered() 

* assert_realloc_entered_count(v) 

  These macros check to see if realloc() has or has not been entered.

  * v = The number of times realloc() has been entered.

* assert_strdup_entered() 

* assert_strdup_not_entered() 

* assert_strdup_entered_count(v) 

  These macros check to see if strdup() has or has not been entered.

  * v = The number of times strdup() has been entered.

## Capture Macros

The capture macros are used when a function that the function under test calls exit() or in similar situations  involving some kind of fatal error. It does not catch exceptions, such as divide by zero or segfault. Capture is enabled by setting the USE_CAPTURE configuration parameter to 1. When capture is enabled **every** function that uses it **must** be placed in a capture box. Otherwise, you will see many strange and unrelated build errors and warnings. This feature should be used in an isolated file and sparingly. 

### Define a Capture Block

To define a capture block, you need to define both the block inside a test as well as a mock. Here is an example:

```c
#define USE_CAPTURE 1
#include "unit_tests.h"

DEF_MOCK(void, abort_program, void)
    RAISE();
END_MOCK
    
DEF_TEST(some_test)
    // do some set-up
    CAPTURE
    	function_that_you_expect_to_abort_program();
	END_CAPTURE
    // When the function in the capture block calls abort_program(), execution will
    // continue after this comment, using longjmp() to jump here directly.
END_TEST
```

There are no asserts to go with this feature, but all of the mock functionality is intact.