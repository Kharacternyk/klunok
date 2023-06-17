inc = include_directories('.')

constants = configuration_data()
constants.set('WATCH_NIX_STORE', get_option('watch_nix_store'))
constants.set('CIRCUIT_BREAKER_SECONDS', get_option('circuit_breaker_seconds'))
constants.set_quoted('VERSION', meson.project_version())
constants.set_quoted('LUA_VERSION', lua.version())
configure_file(output: 'constants.h', configuration: constants)
