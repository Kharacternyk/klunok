inc = include_directories('.')

constants = configuration_data()
constants.set('WATCH_NIX_STORE', get_option('watch_nix_store'))
configure_file(output: 'constants.h', configuration: constants)
