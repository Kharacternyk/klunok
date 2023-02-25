if lua.found()
  lua_split = custom_target(
    output: ['pre_config.lua', 'post_config.lua'],
    input: ['split.lua', 'config.lua'],
    command: [
      'lua', '@INPUT0@', '@INPUT1@', '@OUTPUT0@', '@OUTPUT1@'
    ]
  )
  lua_sources = []
  foreach file: lua_split.to_list()
    lua_sources += custom_target(
      output: file.full_path().split('/')[-1] + '.o',
      input: file,
      command: [
        'ld', '-z', 'noexecstack', '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'
      ]
    )
  endforeach
endif
