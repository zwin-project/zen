_zen_desktop_unit_tests_deps = [
  zen_common_dep,
  zen_dep,
  zen_desktop_dep,
  zen_mock_dep,
  zen_test_harness_dep
]

_zen_desktop_unit_tests = {
  'screen-layout/layout.c': {},
}

foreach path, args : _zen_desktop_unit_tests
  test_name = 'desktop/tests/@0@'.format(path)
  test_env = args.get('env', {})

  bin = executable(
    test_name.underscorify(),
    path,
    dependencies: _zen_desktop_unit_tests_deps,
    include_directories: _zen_desktop_private_inc,
  )

  test(test_name, bin, env: test_env)
endforeach
