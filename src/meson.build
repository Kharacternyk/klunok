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

license_files = []
notices = get_option('dependency_notices')

if notices != ''
  license_files += files(notices / 'intro', notices / 'licenses')
endif

license_files += files('../LICENSE')

license_text = custom_target(
  output: 'licenses.txt',
  input: license_files,
  command: [
    'awk', '-f', files('../awk/notices.awk'),
    '-v', 'source_revision=' + get_option('source_revision'), '@INPUT@'
  ],
  capture: true,
)

license_blob = custom_target(
  output: 'licenses.o',
  input: license_text,
  command: [
    'ld', '-z', 'noexecstack', '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'
  ]
)

main_sources = sources + 'main.c' + license_blob

executable(
  'klunok',
  main_sources,
  dependencies: [
    lua,
    m,
    seccomp,
  ],
  include_directories: inc,
  install: true
)
