dep_zen = [
  dep_libzen_compositor_public,
  dep_zen_shell_public,
]

# TODO: load different backend dynamically
if opt_backend == 'glfw'
  dep_zen += [ dep_glfw_backend_public ]
elif opt_backend == 'openvr'
  dep_zen += [ dep_openvr_backend_public ]
else
  assert(false, f'unsupported backend: @opt_backend@')
endif

src_zen = [
  'main.c'
]

executable(
  'zen',
  src_zen,
  install: true,
  pie: true,
  dependencies: dep_zen,
)
