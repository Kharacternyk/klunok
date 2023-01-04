components = [
  'set',
  'bitmap',
  'deref',
  'store',
  'timestamp',
  'config',
  'main',
]

sources = {}
sources_list = []
foreach component: components
  file = files(component + '.c')[0]
  sources += {component: file}
  sources_list += [file]
endforeach

executable('klunok', sources_list, dependencies: lua, include_directories: inc, install: true)
