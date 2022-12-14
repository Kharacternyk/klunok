project('klunok', 'c')

sources = [
  'src/main.c',
  'src/set.c',
  'src/deref.c',
  'src/config.c',
  'src/store.c',
]

executable('klunok', sources, install: true)

test('set', executable('test-set', ['test/set.c', 'src/set.c']))
test('deref', executable('test-deref', ['test/deref.c', 'src/deref.c']))
test(
  'store',
  executable('test-store', ['test/store.c', 'src/store.c']),
  workdir: meson.current_source_dir() + '/test'
)
