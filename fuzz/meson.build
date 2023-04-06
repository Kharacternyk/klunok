honggfuzz = find_program('honggfuzz', required: false)

if honggfuzz.found()
  fuzz_components = [
    'elfinterp',
  ]

  foreach component: fuzz_components
    fuzz_executable = executable(
      component,
      sources + files(component + '.c'),
      dependencies: [lua, rt, dl],
      include_directories: inc,
    )

    run_target(
      'fuzz-' + component,
      command: [
        honggfuzz,
        '-i',
        '@BUILD_ROOT@/corpus/' + component,
        '-s',
        '--',
        fuzz_executable,
      ],
    )
  endforeach
endif
