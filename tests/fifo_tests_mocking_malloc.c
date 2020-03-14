/*
 *  In these tests, malloc and friends are mocked such that they always return
 *  NULL in order to test the condition where there is not enough memory to
 *  create the data structures.
 */
#define USE_MEMORY 0
#define USE_CAPTURE 1
#define VERBOSE 3
#include "unit_tests.h"

static char *fatal_error_str;
DEF_MOCK(void, fatal_error, const char* fmt, ...)
    fatal_error_str = (char*)fmt;
    RAISE();
END_MOCK

DEF_MOCK(void, MARK, void)
END_MOCK

typedef void *fifo_t;
/*
 *  Include the module directly, without the header. Note that any headers
 *  included by the module under test have to be stubbed out for it to compile.
 */
#include "fifo.c"

/*
 *  Define all of the mocks and stubs before including the module to test.
 */

DEF_MOCK(void*, malloc, size_t size)
    (void)size;
    //printf("here!\n");
    return NULL;
END_MOCK

static char buffer[1024];
static char calloc_pass = 0;
DEF_MOCK(void*, calloc, size_t num, size_t size)
    (void)num;
    (void)size;
    if(calloc_pass != 0) {
        calloc_pass = 0;
        return (void*)buffer;
    }
    else
        return NULL;
END_MOCK

DEF_MOCK(void*, realloc, void* ptr, size_t size)
    (void)ptr;
    (void)size;
    return NULL;
END_MOCK;

DEF_MOCK(void, free, void* ptr)
    (void)ptr;
END_MOCK

DEF_MOCK(char*, strdup, const char* str)
    (void)str;
    return NULL;
END_MOCK;



/*
 *  Define the tests after including the module to test.
 *
 *  The only functions that use memory allocation are
 *      fifo_create()
 *      fifo_destroy()
 *      fifo_add()
 *
 *  The other functions are tested for error paths elsewhere.
 */

DEF_TEST(null_ptr_to_data_returns_error)

    int retv = fifo_get(NULL, NULL, 0);
    assert_int_equal(0, retv);
    assert_mock_not_entered("fatal_error");

    retv = fifo_reset(NULL);
    assert_int_equal(0, retv);
    assert_mock_not_entered("fatal_error");

    // check it twice
    retv = fifo_reset(NULL);
    assert_int_equal(0, retv);
    assert_mock_not_entered("fatal_error");

    CAPTURE
        fifo_add(NULL, NULL, 0);
    END_CAPTURE
    assert_mock_entered("fatal_error");
    assert_mock_entered_count(1, "fatal_error");
    assert_string_equal(fatal_error_str,
                        "attempt to add to an invalid FIFO");
    assert_mock_not_entered("calloc");
    // Note that under gcc, printf() calls malloc the first time it's called.
    // Therefore, this test fails.
    //assert_mock_not_entered("malloc");

END_TEST

DEF_TEST(fifo_create_fails_data_structure)
    void *ptr = NULL;
    CAPTURE
        ptr = fifo_create();
    END_CAPTURE
    assert_mock_entered("fatal_error");
    assert_string_equal(fatal_error_str,
                        "cannot allocate memory for FIFO struct");
    assert_mock_entered("calloc");
    (void)ptr; // make the compiler happy
END_TEST

DEF_TEST(fifo_destroy_does_not_call_free_for_null)
    fifo_destroy(NULL);
    assert_mock_not_entered("free");
END_TEST

DEF_TEST(fifo_add_fatal_error_on_failed_allocate)
    calloc_pass = 1;
    fifo_t ptr;
    CAPTURE
        ptr = fifo_create();
    END_CAPTURE
    assert_ptr_not_null(ptr);
    assert_mock_entered_count(1, "calloc");
    assert_mock_not_entered("fatal_error");

    CAPTURE
        fifo_add(ptr, NULL, 0);
    END_CAPTURE
    assert_mock_entered_count(2, "calloc");
    assert_mock_entered("fatal_error");
    assert_string_equal("cannot allocate memory for FIFO element", fatal_error_str);

    calloc_pass = 2;
    CAPTURE
        fifo_add(ptr, NULL, 0);
    END_CAPTURE
    assert_mock_entered_count(3, "calloc");
    assert_mock_entered_count(1, "malloc");
    assert_mock_entered("fatal_error");
    assert_string_equal("cannot allocate memory for FIFO element data", fatal_error_str);
END_TEST


/*
 *  Define the actual main. There is a lot more to this than you see here. You
 *  can really do anything here that you could do in any other main(), but this
 *  is kept simple intentionally.
 */
DEF_TEST_MAIN("FIFO tests mocking malloc")
    TRACK_MOCK("fatal_error");
    TRACK_MOCK("malloc");
    TRACK_MOCK("calloc");
    TRACK_MOCK("realloc");
    TRACK_MOCK("free");
    TRACK_MOCK("strdup");
    ADD_TEST(null_ptr_to_data_returns_error);
    ADD_TEST(fifo_create_fails_data_structure);
    ADD_TEST(fifo_destroy_does_not_call_free_for_null);
    ADD_TEST(fifo_add_fatal_error_on_failed_allocate);
END_TEST_MAIN
