#pragma once

#ifdef __cplusplus

// NOLINTBEGIN(bugprone-macro-parentheses)
#define DISABLE_MOVE_AND_COPY(Class)        \
  Class(const Class &) = delete;            \
  Class(Class &&) = delete;                 \
  Class &operator=(const Class &) = delete; \
  Class &operator=(Class &&) = delete
// NOLINTEND(bugprone-macro-parentheses)

#endif
