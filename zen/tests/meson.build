subdir('mock')

_zen_unit_tests_deps = [
  zen_dep,
  _zen_deps,
  zen_mock_dep,
  zen_test_harness_dep
]

_zen_unit_tests = {
  'config/config-section.c': {
    'env': {
      'XDG_CONFIG_HOME': '@0@/testdata'.format(meson.current_source_dir()),
    },
  },
  'config/config-path.c': {},
  'inode/inode-map.c': {},
  'inode/inode-move.c': {},
  'server/construct-destroy.c': {},
  'snode/snode-damage.c': {},
  'snode/snode-focus.c': {},
  'snode/snode-frame.c': {},
  'snode/snode-get-at.c': {},
  'snode/snode-position.c': {},
}

foreach path, args : _zen_unit_tests
  test_name = 'zen/tests/@0@'.format(path)
  test_env = args.get('env', {})

  bin = executable(
    test_name.underscorify(),
    path,
    dependencies: _zen_unit_tests_deps,
    include_directories: zen_private_inc,
  )

  test(test_name, bin, env: test_env)
endforeach
