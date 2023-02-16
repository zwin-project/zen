_mock_srcs = [
  'src/output.c',
]

_mock_inc = include_directories('include')

_mock_deps = [
  zen_common_dep,
  zen_dep,
]

_mock_lib = static_library(
  'zen-mock-lib',
  _mock_srcs,
  install: false,
  include_directories: [_mock_inc, zen_private_inc],
  dependencies: _mock_deps,
)

zen_mock_dep = declare_dependency(
  link_with: _mock_lib,
  include_directories: _mock_inc,
)
