_zen_unit_tests = {
  'config/config-section.c': {
    'env': {
      'XDG_CONFIG_HOME': '@0@/testdata'.format(meson.current_source_dir()),
    },
  },
  'config/config-path.c': {},
}

foreach path, args : _zen_unit_tests
  test_name = 'zen/tests/@0@'.format(path)
  test_env = args.get('env', {})

  bin = executable(
    test_name.underscorify(),
    path,
    dependencies: [_zen_deps, zen_test_harness_dep],
    include_directories: zen_private_inc,
  )

  test(test_name, bin, env: test_env)
endforeach
