components = [
  'bitmap',
  'buffer',
  'counter',
  'deref',
  'elfinterp',
  'extension',
  'handler',
  'journal',
  'linq',
  'list',
  'logstep',
  'messages',
  'mountinfo',
  'params',
  'parents',
  'set',
  'sieve',
  'storepath',
  'sync',
  'timestamp',
  'trace',
] + ('config-' + (lua.found() ? 'lua' : 'static'))

sources = lua.found() ? lua_sources : []
foreach component: components
  sources += files(component + '.c')
endforeach
main_sources = sources + 'main.c'

executable(
  'klunok',
  main_sources,
  dependencies: [lua, fts],
  include_directories: inc,
  install: true
)
