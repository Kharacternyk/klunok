honggfuzz = find_program('honggfuzz', required: false)

if honggfuzz.found()
  fuzz_components = [
    'elfinterp',
  ]

  fuzz_sources = main_sources
  foreach component: fuzz_components + 'main'
    fuzz_sources += files(component + '.c')
  endforeach

  fuzz_executable = executable(
    'klunok',
    fuzz_sources,
    dependencies: lua,
    link_args: '-Wl,--export-dynamic',
    include_directories: inc,
  )

  foreach component: fuzz_components
    run_target(
      'fuzz-' + component,
      command: [
        honggfuzz,
        '-x',
        '-i',
        '/bin/', #FIXME
        '-s',
        '--',
        fuzz_executable,
        'fuzz_' + component.underscorify()
      ],
    )
  endforeach
endif
