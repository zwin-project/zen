zen_util_srcs = [
  'log.c'
]

zen_util_lib = static_library(
  'zigen-util',
  zen_util_srcs,
  include_directories: zen_util_inc,
  install: false,
)

zen_util_dep = declare_dependency(
  link_with: zen_util_lib,
  include_directories: zen_util_inc,
)
