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
  if component in ['config-lua', 'store', 'parents', 'linq']
    c_args += '-DTEST_ROOT="' + meson.current_source_dir() + '"'
  endif
  if component.startswith('config-') or component in ['store', 'linq']
    test_sources += sources['parents']
  endif
  if component.startswith('config-')
    test_sources += sources['set']
    test_sources += sources['store']
    test_sources += sources['linq']
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
