#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

struct test {
  const char *name;
  void (*run)(void);
  int must_fail;
} __attribute__((aligned(16)));

#define TEST(name)                                                             \
  static void name##test(void);                                                \
                                                                               \
  const struct test test##name                                                 \
      __attribute__((used, section("test_section"))) = {#name, name##test, 0}; \
                                                                               \
  static void name##test(void)

#endif  //  TEST_RUNNER_H
