fs = import('fs')

components = fs.read('components.few').strip().split()

if lua.found()
  components += 'config-lua'
  sources = lua_sources
else
  components += 'config-static'
  sources = []
endif

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
