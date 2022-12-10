project('klunok', 'c')

executable('klunok', 'src/main.c', install: true)
test('set', executable('test-set', ['test/set.c', 'src/set.c']))
test('deref', executable('test-deref', ['test/deref.c', 'src/deref.c']))
