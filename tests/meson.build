foreach component: components
  if component == 'main'
    continue
  endif

  test_sources = [
    sources[component],
    sources['callback'],
    component + '.c',
  ]
  c_args = []
  dependencies = []

  if component in ['config', 'store']
    c_args = '-DTEST_ROOT="' + meson.current_source_dir() + '"'
  endif
  if component == 'config'
    dependencies += [lua]
    test_sources += sources['set']
  endif

  test(component, executable(
    component,
    test_sources,
    dependencies: dependencies,
    c_args: c_args,
    include_directories: source_dir
  ))
endforeach
