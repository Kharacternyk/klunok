inc = include_directories('.')

constants = configuration_data()
constants.set('WATCH_NIX_STORE', get_option('watch_nix_store'))
constants.set('CIRCUIT_BREAKER_SECONDS', get_option('circuit_breaker_seconds'))
configure_file(output: 'constants.h', configuration: constants)
