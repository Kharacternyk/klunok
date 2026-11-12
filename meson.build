project('klunok', 'c', version: files('./version'))

cc = meson.get_compiler('c')
m = cc.find_library('m', static: true)
dl = cc.find_library('dl')
lua = dependency(get_option('lua'), required: false)

subdir('lua')
subdir('inc')
subdir('src')
subdir('tests')
subdir('fuzz')
