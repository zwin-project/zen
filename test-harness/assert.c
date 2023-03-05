#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test-harness.h"

void
assert_equal_string_(
    const char *actual_expression, char *expected, char *actual)
{
  if (strcmp(expected, actual) != 0) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "Assertion failed: %s\n", actual_expression);
    fprintf(stderr, "\tExpected: %s\n", expected);  // NOLINT(cert-err33-c)
    fprintf(stderr, "\t  Actual: %s\n", actual);    // NOLINT(cert-err33-c)

    exit(EXIT_FAILURE);
  }
}

void
assert_equal_bool_(const char *actual_expression, bool expected, bool actual)
{
  if (expected != actual) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "Assertion failed: %s\n", actual_expression);
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "\tExpected: %s\n", expected ? "true" : "false");
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "\t  Actual: %s\n", actual ? "true" : "false");

    exit(EXIT_FAILURE);
  }
}

void
assert_equal_int_(
    const char *actual_expression, int64_t expected, int64_t actual)
{
  if (expected != actual) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "Assertion failed: %s\n", actual_expression);
    fprintf(stderr, "\tExpected: %ld\n", expected);  // NOLINT(cert-err33-c)
    fprintf(stderr, "\t  Actual: %ld\n", actual);    // NOLINT(cert-err33-c)

    exit(EXIT_FAILURE);
  }
}

void
assert_equal_uint_(
    const char *actual_expression, uint64_t expected, uint64_t actual)
{
  if (expected != actual) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "Assertion failed: %s\n", actual_expression);
    fprintf(stderr, "\tExpected: %ld\n", expected);  // NOLINT(cert-err33-c)
    fprintf(stderr, "\t  Actual: %ld\n", actual);    // NOLINT(cert-err33-c)

    exit(EXIT_FAILURE);
  }
}

void
assert_equal_double_(
    const char *actual_expression, double expected, double actual)
{
  if (expected != actual) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "Assertion failed: %s\n", actual_expression);
    fprintf(stderr, "\tExpected: %f\n", expected);  // NOLINT(cert-err33-c)
    fprintf(stderr, "\t  Actual: %f\n", actual);    // NOLINT(cert-err33-c)

    exit(EXIT_FAILURE);
  }
}

void
assert_equal_pointer_(
    const char *actual_expression, void *expected, void *actual, bool not )
{
  if ((expected == actual) == not ) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "Assertion failed: %s\n", actual_expression);
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "\tExpected: %s %p\n", not ? "not" : "", expected);
    fprintf(stderr, "\t  Actual: %p\n", actual);  // NOLINT(cert-err33-c)

    exit(EXIT_FAILURE);
  }
}

void
assert_equal_vec_(const char *actual_expression, const float *expected,
    const float *actual, int count)
{
  bool ok = true;
  for (int i = 0; i < count; i++) {
    if (expected[i] != actual[i]) {
      ok = false;
    }
  }

  if (!ok) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "Assertion failed: %s\n", actual_expression);

    fprintf(stderr, "\tExpected: (%f", expected[0]);  // NOLINT(cert-err33-c)
    for (int i = 1; i < count; i++) {
      fprintf(stderr, ", %f", expected[i]);  // NOLINT(cert-err33-c)
    }
    fprintf(stderr, ")\n");  // NOLINT(cert-err33-c)

    fprintf(stderr, "\t  Actual: (%f", actual[0]);  // NOLINT(cert-err33-c)
    for (int i = 1; i < count; i++) {
      fprintf(stderr, ", %f", actual[i]);  // NOLINT(cert-err33-c)
    }
    fprintf(stderr, ")\n");  // NOLINT(cert-err33-c)

    exit(EXIT_FAILURE);
  }
}
