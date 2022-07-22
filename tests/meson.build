_libxml_dep = dependency('libxml-2.0')

_wayland_dtd_path = join_paths(wayland_scanner_dep.get_variable('pkgdatadir'), 'wayland.dtd')

_tests = {
  'validate-xml': {
    'env': {
      'WAYLAND_DTD_PATH': _wayland_dtd_path,
      'ZEN_DESKTOP_XML': zen_desktop_xml_abs_path,
    },
    'deps': _libxml_dep,
  },
}

foreach test_name, args : _tests
  test_sources = [ test_name + '-test.c', 'test-runner.c']
  test_env = args.get('env', [])
  test_deps = args.get('deps', [])

  bin = executable(
    test_name,
    test_sources,
    dependencies: test_deps,
  )

  test(test_name, bin, env: test_env)
endforeach
