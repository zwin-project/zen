dep_zen = [
  dep_libzen_compositor_public,
  dep_glfw_backend_public, # TODO: load different backend dynamically
  dep_zen_shell_public,
]

src_zen = [
  'main.c'
]

executable(
  'zen',
  src_zen,
  install: true,
  dependencies: dep_zen,
)
