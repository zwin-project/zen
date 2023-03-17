_zen_common_unit_tests_deps = [
  _zen_common_deps,
  zen_common_dep,
  zen_test_harness_dep
]

_zen_common_unit_tests = {
  'color-hsl.c': {},
}

foreach path, args : _zen_common_unit_tests
  test_name = 'commont/tests/@0@'.format(path)
  test_env = args.get('env', {})

  bin = executable(
    test_name.underscorify(),
    path,
    dependencies: _zen_common_unit_tests_deps,
    include_directories: _zen_common_private_inc,
  )

  test(test_name, bin, env: test_env)
endforeach
