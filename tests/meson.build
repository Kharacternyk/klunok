foreach component: components
  test_sources = [
    sources[component],
    component + '.c',
  ]
  c_args = []
  dependencies = []

  if component == 'config-lua' and not lua.found()
    continue
  endif
  if component in ['config-lua', 'store']
    c_args += '-DTEST_ROOT="' + meson.current_source_dir() + '"'
  endif
  if component in ['config-lua', 'config-static']
    test_sources += sources['set']
    test_sources += sources['store']
    test_sources += 'config.c'
  endif
  if component == 'config-lua'
    dependencies += lua
    test_sources += lua_sources
  endif

  test(component, executable(
    component,
    test_sources,
    dependencies: dependencies,
    c_args: c_args,
    include_directories: inc
  ))
endforeach
