add_test_setup('fast')
add_test_setup('gdb', gdb: true)

valgrind = find_program('valgrind', required: false)

if valgrind.found()
  add_test_setup(
    'valgrind',
    exe_wrapper: [valgrind, '--error-exitcode=1', '--leak-check=full'],
    is_default: true
  )
endif

test_sources = main_sources
foreach component: components + 'config'
  test_sources += files(component + '.c')
endforeach

test_executable = executable(
  'klunok',
  test_sources,
  dependencies: lua,
  c_args: '-DTEST_ROOT="' + meson.current_source_dir() + '"',
  link_args: '-Wl,--export-dynamic',
  include_directories: inc,
)

foreach component: components
  test(
    component,
    test_executable,
    args: 'test_' + component.underscorify(),
    should_fail: component == 'circbreak' or component == 'handler' and not lua.found(),
  )
endforeach
