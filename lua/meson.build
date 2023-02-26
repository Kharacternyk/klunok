if lua.found()
  lua_split = custom_target(
    output: ['pre_config.lua', 'post_config.lua', 'doc.lua'],
    input: ['split.lua', 'config.lua'],
    command: [
      'lua', '@INPUT0@', '@INPUT1@', '@OUTPUT0@', '@OUTPUT1@', '@OUTPUT2@'
    ]
  )
  lua_sources = []
  foreach file: lua_split.to_list()
    filename = file.full_path().split('/')[-1]
    if filename == 'doc.lua'
      lua_doc = custom_target(
        output: 'doc',
        input: ['config.ld', file],
        command: [
          'ldoc', '-c', '@INPUT0@', '-d', '@OUTPUT@', '@INPUT1@'
        ]
      )
    else
      lua_sources += custom_target(
        output: filename + '.o',
        input: file,
        command: [
          'ld', '-z', 'noexecstack', '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'
        ]
      )
    endif
  endforeach
endif
