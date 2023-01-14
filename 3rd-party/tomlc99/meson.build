_toml_inc = include_directories('include')

_toml_lib = static_library(
  'tomlc99',
  'toml.c',
  install: false,
  include_directories: _toml_inc,
)

toml_dep = declare_dependency(
  link_with: _toml_lib,
  include_directories: _toml_inc,
)
