components = [
  'set',
  'bitmap',
  'deref',
  'store',
  'timestamp',
  'config-lua',
  'config-static',
]

sources = {}
main_sources = ['main.c']

foreach component: components
  file = files(component + '.c')
  sources += {component: file}
  if not component.startswith('config-')
    main_sources += file
  endif
endforeach

if lua.found()
  main_sources += lua_sources
  main_sources += sources['config-lua']
else
  main_sources += sources['config-static']
endif

executable('klunok', main_sources, dependencies: lua, include_directories: inc, install: true)
