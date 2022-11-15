_libxml_dep = dependency('libxml-2.0')
_shell_check = find_program('shellcheck')

_wayland_dtd_path = join_paths(wayland_scanner_dep.get_variable('pkgdatadir'), 'wayland.dtd')
_test_inc = include_directories('.')

_tests = {
  'protocol/validate-xml': {
    'env': {
      'WAYLAND_DTD_PATH': _wayland_dtd_path,
      'ZEN_DESKTOP_XML': zen_desktop_xml_abs_path,
    },
    'deps': _libxml_dep,
  },
}

foreach key, args : _tests
  test_name = key.underscorify()
  test_sources = [ key + '-test.c', 'test-runner.c']
  test_env = args.get('env', [])
  test_deps = args.get('deps', [])

  bin = executable(
    test_name,
    test_sources,
    dependencies: test_deps,
    include_directories: _test_inc,
  )

  test(test_name, bin, env: test_env)
endforeach

_posix_shell_scripts = [
  textify,
]

foreach shell : _posix_shell_scripts
  test('shellcheck @0@'.format(shell), _shell_check, args: shell)
endforeach
