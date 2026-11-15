add_test_setup('fast', exclude_suites: ['valgrind'])
add_test_setup('gdb', gdb: true, exclude_suites: ['valgrind'])

valgrind = find_program('valgrind', required: false)

if valgrind.found()
  add_test_setup(
    'valgrind',
    exclude_suites: ['fast'],
    is_default: true
  )
endif

test_constants = configuration_data()
test_constants.set_quoted('TEST_ROOT', meson.current_source_dir())
configure_file(output: 'test-constants.h', configuration: test_constants)

test_sources = main_sources
foreach component: components + 'config'
  test_sources += files(component + '.c')
endforeach

test_executable = executable(
  'klunok',
  test_sources,
  dependencies: [
    lua,
    dl,
    m,
    seccomp,
  ],
  link_args: '-Wl,--export-dynamic',
  include_directories: inc,
)

foreach component: components
  test_function = 'test_' + component.underscorify()
  should_fail = component == 'handler' and not lua.found()

  test(
    component == 'execmem' ? component : component + '-fast',
    test_executable,
    args: test_function,
    suite: component == 'execmem' ? 'execmem' : 'fast',
    should_fail: should_fail,
  )

  if valgrind.found() and component != 'execmem'
    test(
      component,
      valgrind,
      args: ['--error-exitcode=1', '--leak-check=full', test_executable, test_function],
      suite: 'valgrind',
      should_fail: should_fail,
    )
  endif
endforeach
