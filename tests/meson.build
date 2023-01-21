add_test_setup('fast')
add_test_setup('gdb', gdb: true)
add_test_setup(
  'valgrind',
  exe_wrapper: ['valgrind', '--error-exitcode=1', '--leak-check=full'],
  is_default: true
)

foreach component: components
  test_sources = [
    sources[component],
    sources['trace'],
    sources['messages'],
    component + '.c',
  ]
  c_args = []
  dependencies = []
  should_fail = false

  if component == 'messages'
    continue
  endif
  if component == 'config-lua' and not lua.found()
    continue
  endif
  if component == 'circbreak'
    should_fail = true
  endif
  if component in ['config-lua', 'store', 'parents', 'linq', 'handler', 'elfinterp']
    c_args += '-DTEST_ROOT="' + meson.current_source_dir() + '"'
  endif
  if component in ['deref', 'linq', 'store']
    test_sources += sources['builder']
  endif
  if component.startswith('config-') or component in ['store', 'linq']
    test_sources += sources['parents']
  endif
  if component.startswith('config-')
    test_sources += sources['set']
    test_sources += 'config.c'
  endif
  if component == 'config-lua'
    test_sources += sources['circbreak']
    dependencies += lua
    test_sources += lua_sources
  endif
  if component == 'handler'
    foreach _component: components
      if _component == 'config-lua'
        if lua.found()
          dependencies += lua
          test_sources += sources[_component]
          test_sources += lua_sources
        endif
      elif _component == 'config-static'
        if not lua.found()
          test_sources += sources[_component]
        endif
      else
        test_sources += sources[_component]
      endif
    endforeach
  endif

  test(
    component,
    executable(
      component,
      test_sources,
      dependencies: dependencies,
      c_args: c_args,
      include_directories: inc,
    ),
    should_fail: should_fail,
  )
endforeach
