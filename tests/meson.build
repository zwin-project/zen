tests = {
  'noop-test': {},
  'load-backend': {
    'deps': [
      dep_headless_backend_public,
      dep_libzen_compositor_private,
      dep_oepngl_renderer_public,
      dep_zen_shell_public
    ],
  },
  'virtual-object-role-test': {},
  'cglm-wl-array-test': {
    'deps': [
      dep_libzen_compositor_public,
    ]
  }
}

foreach test_name, args : tests
  test_sources = [ test_name + '.c', 'test-runner.c' ]
  bin = executable(
    test_name,
    test_sources,
    dependencies: args.get('deps', []),
  )
  test(test_name, bin)
endforeach
