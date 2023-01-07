if lua.found()
  ld = find_program('ld')
  lua_sources = []
  foreach file: ['config.lua', 'validation.lua']
    copy = custom_target(
      output: file,
      input: file,
      command: [
        'cp', '@INPUT@', '@OUTPUT@'
      ],
    )
    lua_sources += custom_target(
      file + '.o',
      output: file + '.o',
      input: copy,
      command: [
        ld, '-z', 'noexecstack', '-r', '-b', 'binary', '-o', '@OUTPUT@', '@INPUT@'
      ]
    )
  endforeach
endif
