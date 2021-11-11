tests = {
  'noop-test': {},
}

foreach test_name, args : tests
  test_sources = [ test_name + '.c', 'test-runner.c' ]
  bin = executable(test_name, test_sources)
  test(test_name, bin)
endforeach
