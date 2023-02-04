-- Reloadable settings
store_root = './klunok/store'
queue_path = './klunok/queue'
version_pattern = 'v%Y-%m-%d-%H-%M'
debounce_seconds = 60
path_length_guess = 1024
queue_size_guess = nil -- (debounce_seconds * 2)
editors = {
  atom = true,
  code = true,
  codium = true,
  gedit = true,
  howl = true,
  hx = true,
  inkscape = true,
  kak = true,
  kate = true,
  kwrite = true,
  micro = true,
  nano = true,
  nvim = true,
  pluma = true,
  rsession = true,
  sublime_text = true,
  vi = true,
  vim = true,
  xed = true,
  ["gnome-text-editor"] = true,
  ["notepadqq-bin"] = true,
  ["soffice.bin"] = true,
  [".gedit-wrapped"] = true,
  [".gnome-text-editor-wrapped"] = true,
  [".howl-wrapped"] = true,
  [".hx-wrapped"] = true,
  [".inkscape-wrapped"] = true,
  [".kate-wrapped"] = true,
  [".kwrite-wrapped"] = true,
  [".pluma-wrapped"] = true,
  [".xed-wrapped"] = true,
}

-- Not reloadable settings
max_pid_guess = 2^15
elf_interpreter_count_guess = 1
executable_count_guess = 128

-- Compile-time settings
circuit_breaker_seconds = 5
