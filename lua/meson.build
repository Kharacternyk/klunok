if lua.found()
  lua_split = custom_target(
    output: ['pre_config.lua', 'post_config.lua'],
    input: 'config.lua.md',
    command: [
      'awk', '-f', files('../awk/weave-lua-config.awk'),
      '-vpre=@OUTPUT0@',
      '-vpost=@OUTPUT1@',
      '@INPUT@'
    ]
  )
  lua_sources = []
  foreach file: lua_split.to_list()
    filename = file.full_path().split('/')[-1]
    lua_sources += custom_target(
      output: filename + '.o',
      input: file,
      command: [
        'ld', '-z', 'noexecstack', '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'
      ]
    )
  endforeach
endif
