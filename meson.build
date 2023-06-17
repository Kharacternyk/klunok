project('klunok', 'c', version: files('./version'))

cc = meson.get_compiler('c')
rt = cc.find_library('rt')
dl = cc.find_library('dl')
fts = dependency('musl-fts', required: false)
lua = dependency(get_option('lua'), required: false)

subdir('lua')
subdir('inc')
subdir('src')
subdir('tests')
subdir('fuzz')
