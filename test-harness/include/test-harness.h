#pragma once

#include <stdbool.h>
#include <stdint.h>

struct test {
  const char *name;
  void (*run)(void);
  int must_fail;
} __attribute__((aligned(16)));

/**
 * Add a test.
 *
 * This defines one test as a new function. This newly defined function is
 * called in the forked process.
 *
 * The test fails if the process exits with a non-zero exit status in this
 * function. Otherwise, the test succeeds.
 */
#define TEST(name)                                               \
  static void name##_test(void);                                 \
                                                                 \
  const struct test test##name __attribute__((                   \
      used, section("test_section"))) = {#name, name##_test, 0}; \
                                                                 \
  static void name##_test(void)

// Assertions

void assert_equal_string_(
    const char *actual_expression, char *expected, char *actual);

void assert_equal_bool_(
    const char *actual_expression, bool expected, bool actual);

void assert_equal_int_(
    const char *actual_expression, int64_t expected, int64_t actual);

void assert_equal_uint_(
    const char *actual_expression, uint64_t expected, uint64_t actual);

void assert_equal_pointer_(
    const char *actual_expression, void *expected, void *actual);

#define ASSERT_EQUAL_STRING(expected, actual) \
  assert_equal_string_(#actual, expected, actual)

#define ASSERT_EQUAL_BOOL(expected, actual) \
  assert_equal_bool_(#actual, expected, actual)

#define ASSERT_EQUAL_INT(expected, actual) \
  assert_equal_int_(#actual, expected, actual)

#define ASSERT_EQUAL_UINT(expected, actual) \
  assert_equal_int_(#actual, expected, actual)

#define ASSERT_EQUAL_POINTER(expected, actual) \
  assert_equal_pointer_(#actual, expected, actual)
