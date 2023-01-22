store_root = './klunok/store'
queue_path = './klunok/queue'
version_pattern = 'v%Y-%m-%d-%H-%M'
debounce_seconds = 60
version_max_length = 80
path_length_guess = 1024
max_pid_guess = 2^16
elf_interpreter_count_guess = 1
executable_count_guess = 128
editors = {
  vi = true,
  vim = true,
  nvim = true,
  rsession = true,
  ["soffice.bin"] = true,
}

circuit_breaker_seconds = 5
