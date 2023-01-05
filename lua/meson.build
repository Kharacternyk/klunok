ld = find_program('ld')

lua_config_copy = custom_target(
  output: 'config.lua',
  input: 'config.lua',
  command: [
    'cp', '@INPUT@', '@OUTPUT@'
  ],
)

lua_config_object = custom_target(
  'config.lua.o',
  output: 'config.lua.o',
  input: lua_config_copy,
  command: [
    ld, '-z', 'noexecstack', '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'
  ]
)
