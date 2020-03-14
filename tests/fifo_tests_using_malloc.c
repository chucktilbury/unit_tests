
/*
 *  This tests shows that the fifo create and the fifo destroy functions behave
 *  in the expected manner when the memory allocate functions are enabled. The
 *  assumption is made that there is enough available memory to allocate the
 *  data structures.
 */
#define USE_MEMORY 1
#define VERBOSE 1
#include "unit_tests.h"

/*
 *  Define symbols so that the module under test can actually link.
 */
typedef void* fifo_t;

DEF_MOCK(void, MARK, void)
    // normally, this would print a message. Here it does nothing.
END_MOCK

DEF_MOCK(void, fatal_error, char *str, ...)
    // Normally, this function prints an error and kills the program. Here it
    // does nothing.
    (void)str;
    //exit(1);
END_MOCK

/*
 *  Include the module directly, without the header. Note that any headers
 *  included by the module under test have to be stubbed out for it to compile.
 */
#include "fifo.c"

/*
 *  Define tests.
 */
DEF_TEST(create_fifo_and_destroy_fifo_succeed)
    fifo_t ptr = fifo_create();
    assert_memory_pool_size(32);
    assert_calloc_entered_count(1);

    fifo_destroy(ptr);
    assert_memory_pool_size(0);
    assert_free_entered_count(1);

    assert_mock_not_entered("fatal_error");
END_TEST

DEF_TEST(empty_fifo_returns_error_on_get)
    fifo_t ptr = fifo_create();
    assert_memory_pool_size(32);
    assert_calloc_entered_count(1);

    int value = 123;
    int retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_int_equal(123, value);
    assert_int_equal(0, retv);

    fifo_destroy(ptr);
    assert_memory_pool_size(0);
    assert_free_entered_count(1);

    assert_mock_not_entered("fatal_error");
END_TEST

DEF_TEST(empty_list_reset_no_error)
    fifo_t ptr = fifo_create();
    assert_memory_pool_size(32);
    assert_calloc_entered_count(1);

    int value = 123;
    int retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_int_equal(123, value);
    assert_int_equal(0, retv);

    retv = fifo_reset(ptr);
    assert_int_equal(1, retv);

    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_int_equal(123, value);
    assert_int_equal(0, retv);

    retv = fifo_reset(ptr);
    assert_int_equal(1, retv);

    fifo_destroy(ptr);
    assert_memory_pool_size(0);
    assert_free_entered_count(1);

    assert_mock_not_entered("fatal_error");
END_TEST

DEF_TEST(fifo_items_are_returned_in_order)
    fifo_t ptr = fifo_create();
    assert_memory_pool_size(32);
    assert_memory_total_size(64);
    assert_calloc_entered_count(1);

    int value = 1;
    fifo_add(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(60);
    assert_calloc_entered_count(2);
    assert_malloc_entered_count(1);

    value = 2;
    fifo_add(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(88);
    assert_calloc_entered_count(3);
    assert_malloc_entered_count(2);

    value = 3;
    fifo_add(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(116);
    assert_calloc_entered_count(4);
    assert_malloc_entered_count(3);

    int retv = 0;
    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(116);
    assert_int_equal(1, value);
    assert_int_equal(1, retv);
    assert_calloc_entered_count(4);
    assert_malloc_entered_count(3);

    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(116);
    assert_int_equal(2, value);
    assert_int_equal(1, retv);
    assert_calloc_entered_count(4);
    assert_malloc_entered_count(3);

    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(116);
    assert_int_equal(3, value);
    assert_int_equal(1, retv);
    assert_calloc_entered_count(4);
    assert_malloc_entered_count(3);

    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(116);
    assert_int_equal(0, retv);
    assert_calloc_entered_count(4);
    assert_malloc_entered_count(3);

    fifo_destroy(ptr);
    assert_memory_pool_size(0);
    assert_free_entered_count(7);

    assert_mock_not_entered("fatal_error");
END_TEST

DEF_TEST(single_item_returns_after_reset)
    fifo_t ptr = fifo_create();
    assert_memory_pool_size(32);
    assert_calloc_entered_count(1);

    int value = 123;
    fifo_add(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(60);

    int retv = 0;
    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(60);
    assert_int_equal(123, value);
    assert_int_equal(1, retv);

    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(60);
    assert_int_equal(123, value);
    assert_int_equal(0, retv);

    retv = fifo_reset(ptr);
    assert_int_equal(1, retv);

    retv = 0;
    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(60);
    assert_int_equal(123, value);
    assert_int_equal(1, retv);

    retv = fifo_get(ptr, (void*)&value, sizeof(int));
    assert_memory_pool_size(60);
    assert_int_equal(123, value);
    assert_int_equal(0, retv);

    fifo_destroy(ptr);
    assert_memory_pool_size(0);
    assert_free_entered_count(3);

END_TEST

/*
 *  Define the actual main. There is a lot more to this than you see here. You
 *  can really do anything here that you could do in any other main(), but this
 *  is kept simple intentionally.
 */
DEF_TEST_MAIN("FIFO tests using malloc")
    TRACK_MOCK("fatal_error");
    ADD_TEST(create_fifo_and_destroy_fifo_succeed);
    ADD_TEST(fifo_items_are_returned_in_order);
    ADD_TEST(empty_fifo_returns_error_on_get);
    ADD_TEST(single_item_returns_after_reset);
    ADD_TEST(empty_list_reset_no_error);
END_TEST_MAIN
