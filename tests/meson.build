foreach component: components
  if component == 'main'
    continue
  endif

  test_sources = [
    sources[component],
    component + '.c',
  ]
  c_args = []
  dependencies = []

  if component in ['config', 'store']
    c_args += '-DTEST_ROOT="' + meson.current_source_dir() + '"'
  endif
  if component == 'config'
    dependencies += lua
    test_sources += sources['set']
    test_sources += lua_config_object
  endif

  test(component, executable(
    component,
    test_sources,
    dependencies: dependencies,
    c_args: c_args,
    include_directories: inc
  ))
  run_target('valgrind-' + component, command: ['valgrind.sh', component])
endforeach

run_target('valgrind', command: 'valgrind.sh')
