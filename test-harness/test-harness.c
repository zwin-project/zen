#include "test-harness.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl51-cpp,cert-dcl37-c)
extern const struct test __start_test_section, __stop_test_section;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool is_atty = false;

enum output_color {
  RED = 0,
  GREEN,
  DEFAULT,
  COLOR_COUNT,
};

static const char *const color_curses[COLOR_COUNT] = {
    [RED] = "\033[31m",
    [GREEN] = "\033[32m",
    [DEFAULT] = "\033[0m",
};

static void color_print(enum output_color color, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

static void
color_print(enum output_color color, const char *format, ...)
{
  if (is_atty) {
    fprintf(stderr, "%s", color_curses[color]);  // NOLINT(cert-err33-c)
  }

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);  // NOLINT(cert-err33-c)
  va_end(args);

  if (is_atty) {
    fprintf(stderr, "%s", color_curses[DEFAULT]);  // NOLINT(cert-err33-c)
  }
}

static void
run_test(const struct test *t)
{
  t->run();

  exit(EXIT_SUCCESS);
}

/// @return true if test passes
bool
asses_test(const struct test *t, pid_t pid)
{
  siginfo_t info;
  bool success = false;

  if (waitid(P_PID, pid, &info, WEXITED)) {
    color_print(RED, "waitid failed: %s\n", strerror(errno));

    abort();
  }

  switch (info.si_code) {
    case CLD_EXITED:
      if (info.si_status == EXIT_SUCCESS) {
        success = !t->must_fail;
      } else {
        success = t->must_fail;
      }

      color_print(success ? GREEN : RED, "test \"%s\": \texit status %d",
          t->name, info.si_status);
      break;

    case CLD_KILLED:
    case CLD_DUMPED:
      if (t->must_fail) {
        success = 1;
      }

      color_print(success ? GREEN : RED, "test \"%s\":\tsignal %d", t->name,
          info.si_status);
      break;
  }

  if (success) {
    color_print(GREEN, ", pass.\n");
  } else {
    color_print(RED, ", fail.\n");
  }

  return success;
}

int
main(void)
{
  const struct test *t = NULL;
  pid_t pid = 0;
  int total = 0;
  int pass = 0;

  if (isatty(fileno(stderr))) {
    is_atty = true;
  }

  pass = 0;
  for (t = &__start_test_section; t < &__stop_test_section; t++) {
    pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
      run_test(t); /* never returns */
    }

    if (asses_test(t, pid)) {
      pass++;
    }

    color_print(DEFAULT, "-----------------------------------\n");
  }

  total = (int)(&__stop_test_section - &__start_test_section);
  color_print(
      DEFAULT, "%d tests, %d pass, %d fail\n", total, pass, total - pass);

  return pass == total ? EXIT_SUCCESS : EXIT_FAILURE;
}
