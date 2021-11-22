tests = {
  'noop-test': {},
  'load-backend': {
    'deps': [ dep_libzen_private, dep_headless_backend_public ],
  },
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
