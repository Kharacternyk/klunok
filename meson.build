project('klunok', 'c')

components = {
  'callback': [],
  'set': ['callback'],
  'bitmap': ['callback'],
  'deref': ['callback'],
  'store': ['callback'],
  'timestamp': ['callback'],
  'filelines': ['set', 'callback'],
}

sources = {}
all_sources = []
foreach component: components.keys() + ['main']
  source = 'src/' + component + '.c'
  sources += {component: source}
  all_sources += [source]
endforeach

executable('klunok', all_sources, install: true)

foreach component, dependencies: components
  test_sources = ['test/' + component + '.c', sources[component]]
  foreach test_dependency: dependencies
    test_sources += sources[test_dependency]
  endforeach

  c_args = '-DPROJECT_ROOT="' + meson.current_source_dir() + '"'

  test(component, executable('test-' + component, test_sources, c_args: c_args))
endforeach
