_toml_inc = include_directories('include')

_toml_lib = static_library(
  'tomlc17',
  'tomlc17.c',
  install: false,
  include_directories: _toml_inc,
  override_options: [ 'c_std=c17' ],
)

toml_dep = declare_dependency(
  link_with: _toml_lib,
  include_directories: _toml_inc,
)
