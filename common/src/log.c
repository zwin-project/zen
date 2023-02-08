#include "zen-common/log.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "zen-common/timespec-util.h"
#include "zen-common/util.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static terminate_callback_t log_terminate = exit;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static zn_log_importance_t log_importance = ZEN_ERROR;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static struct timespec start_time = {-1, -1};

static const char *const verbosity_colors[] = {
    [ZEN_SILENT] = "",
    [ZEN_ERROR] = "\x1B[1;31m",
    [ZEN_WARN] = "\x1B[1;33m",
    [ZEN_INFO] = "\x1B[1;34m",
    [ZEN_DEBUG] = "\x1B[1;30m",
};

static const char *const verbosity_headers[] = {
    [ZEN_SILENT] = "",
    [ZEN_ERROR] = "[ERROR]",
    [ZEN_WARN] = "[WARNING]",
    [ZEN_INFO] = "[INFO]",
    [ZEN_DEBUG] = "[DEBUG]",
};

static void
init_start_time(void)
{
  if (start_time.tv_sec >= 0) {
    return;
  }
  clock_gettime(CLOCK_MONOTONIC, &start_time);
}

static void
zn_log_stderr(zn_log_importance_t verbosity, const char *fmt, va_list args)
{
  struct timespec time_since_start = {0};
  unsigned verbosity_index = 0;

  init_start_time();

  if (verbosity > log_importance) {
    return;
  }

  clock_gettime(CLOCK_MONOTONIC, &time_since_start);
  timespec_sub(&time_since_start, &time_since_start, &start_time);

  // NOLINTNEXTLINE(cert-err33-c)
  fprintf(stderr, "%02d:%02d:%02d.%03ld ",
      (int)(time_since_start.tv_sec / 60 / 60),
      (int)(time_since_start.tv_sec / 60 % 60),
      (int)(time_since_start.tv_sec % 60), time_since_start.tv_nsec / 1000000);

  verbosity_index = ZN_MIN(verbosity, ZEN_LOG_IMPORTANCE_LAST - 1);

  if (isatty(STDERR_FILENO)) {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "%s", verbosity_colors[verbosity_index]);
  } else {
    // NOLINTNEXTLINE(cert-err33-c)
    fprintf(stderr, "%s ", verbosity_headers[verbosity_index]);
  }

  vfprintf(stderr, fmt, args);  // NOLINT(cert-err33-c)

  if (isatty(STDERR_FILENO)) {
    fprintf(stderr, "\x1B[0m");  // NOLINT(cert-err33-c)
  }

  fprintf(stderr, "\n");  // NOLINT(cert-err33-c)
}

void
zn_log_init(zn_log_importance_t verbosity, terminate_callback_t terminate)
{
  init_start_time();

  if (verbosity < ZEN_LOG_IMPORTANCE_LAST) {
    log_importance = verbosity;
  }

  if (terminate) {
    log_terminate = terminate;
  }
}

void
zn_log_(zn_log_importance_t verbosity, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  zn_log_stderr(verbosity, format, args);
  va_end(args);
}

void
zn_vlog_(zn_log_importance_t verbosity, const char *format, va_list args)
{
  zn_log_stderr(verbosity, format, args);
}

void
zn_abort_(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  zn_vlog_(ZEN_ERROR, format, args);
  va_end(args);
  log_terminate(EXIT_FAILURE);
}

bool
zn_assert_(bool condition, const char *format, ...)
{
  if (condition) {
    return true;
  }

  va_list args;
  va_start(args, format);
  zn_vlog_(ZEN_ERROR, format, args);
  va_end(args);

#ifndef NDEBUG
  raise(SIGABRT);  // NOLINT(cert-err33-c)
#endif

  return false;
}
