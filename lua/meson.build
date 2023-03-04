if lua.found()
  lua_split = custom_target(
    output: ['pre_config.lua', 'post_config.lua', 'doc.lua'],
    input: ['split.awk', 'config.lua'],
    command: [
      'awk', '-f@INPUT0@',
      '-vpre=@OUTPUT0@',
      '-vpost=@OUTPUT1@',
      '-vdoc=@OUTPUT2@',
      '@INPUT1@'
    ]
  )
  lua_sources = []
  foreach file: lua_split.to_list()
    filename = file.full_path().split('/')[-1]
    if filename == 'doc.lua'
      if get_option('build_doc')
        custom_target(
          output: 'doc',
          input: ['ldoc.lua', file],
          command: [
            'ldoc', '-c', '@INPUT0@', '-d', '@OUTPUT@', '@INPUT1@'
          ],
          install: true,
          install_dir: 'share/klunok/lua',
          install_tag: 'doc',
        )
      endif
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
