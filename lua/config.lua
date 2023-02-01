-- Reloadable settings
store_root = './klunok/store'
queue_path = './klunok/queue'
version_pattern = 'v%Y-%m-%d-%H-%M'
debounce_seconds = 60
path_length_guess = 1024
queue_size_guess = nil -- (debounce_seconds * 2)
editors = {
  vi = true,
  vim = true,
  nvim = true,
  code = true,
  codium = true,
  kak = true,
  micro = true,
  nano = true,
  atom = true,
  sublime_text = true,
  hx = true,
  [".hx-wrapped"] = true,
  kate = true,
  [".kate-wrapped"] = true,
  kwrite = true,
  [".kwrite-wrapped"] = true,
  pluma = true,
  [".pluma-wrapped"] = true,
  xed = true,
  [".xed-wrapped"] = true,
  gedit = true,
  [".gedit-wrapped"] = true,
  ["gnome-text-editor"] = true,
  [".gnome-text-editor-wrapped"] = true,
  ["notepadqq-bin"] = true,
  howl = true,
  [".howl-wrapped"] = true,
  rsession = true,
  ["soffice.bin"] = true,
  inkscape = true,
  [".inkscape-wrapped"] = true,
}

-- Not reloadable settings
max_pid_guess = 2^16
elf_interpreter_count_guess = 1
executable_count_guess = 128

-- Compile-time settings
circuit_breaker_seconds = 5
