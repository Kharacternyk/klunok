run_target('valgrind', command: 'valgrind.sh')

foreach component: components
  run_target('valgrind-' + component, command: ['valgrind.sh', component])
endforeach
