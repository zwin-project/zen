_zen_test_harness_inc = include_directories('include')

_zen_test_harness_lib = static_library(
  'zen-test-harness',
  ['test-harness.c', 'assert.c'],
  install: false,
  include_directories: _zen_test_harness_inc,
)

zen_test_harness_dep = declare_dependency(
  link_with: _zen_test_harness_lib,
  include_directories: _zen_test_harness_inc,
)
