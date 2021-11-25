deps_headless_backend = [
  dep_libzen_public,
]

srcs_headless_backend = [
  'backend.c',
  'output.c',
]

lib_headless_backend = static_library(
  'headless-backend',
  srcs_headless_backend,
  install: false,
  dependencies: deps_headless_backend,
)

dep_headless_backend_public = declare_dependency(
  link_with: lib_headless_backend,
  include_directories: public_inc,
)
