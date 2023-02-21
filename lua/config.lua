-- Helpers
prefix = './klunok'
--[[
prefix_var = prefix .. '/var'
--]]

-- Reloadable settings
--[[
store_root = prefix .. '/store'
queue_path = prefix_var .. '/queue'
journal_path = prefix_var .. '/journal'
--]]
journal_timestamp_pattern = '%Y-%m-%d-%H-%M'
--[[
version_pattern = 'v' .. journal_timestamp_pattern
--]]
debounce_seconds = 60
--[[
queue_size_guess = debounce_seconds * 2
--]]
path_length_guess = 1024
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
  ['gnome-text-editor'] = true,
  ['notepadqq-bin'] = true,
  ['soffice.bin'] = true,
  ['.gedit-wrapped'] = true,
  ['.gnome-text-editor-wrapped'] = true,
  ['.howl-wrapped'] = true,
  ['.hx-wrapped'] = true,
  ['.inkscape-wrapped'] = true,
  ['.kate-wrapped'] = true,
  ['.kwrite-wrapped'] = true,
  ['.pluma-wrapped'] = true,
  ['.xed-wrapped'] = true,
}
event_open_exec_not_editor = nil
event_open_exec_editor = nil
event_open_exec_interpreter = nil
event_close_write_not_by_editor = nil
event_close_write_by_editor = nil
event_queue_head_deleted = nil
event_queue_head_forbidden = nil
event_queue_head_stored = ""

-- Not reloadable settings
max_pid_guess = 2^15
elf_interpreter_count_guess = 1
executable_count_guess = 128

-- Compile-time settings
circuit_breaker_seconds = 5
