--- Default configuration values

--_
function is_string(name)
  assert(type(_G[name]) == 'string', name .. ' must be a string')
end

function is_nil_or_string(name)
  assert(_G[name] == nil or type(_G[name]) == 'string', name .. ' must be nil or a string')
end

function is_positive(name)
  local value = _G[name]
  assert(
    type(value) == 'number' and math.floor(value) == value and value >= 0,
    name .. ' must be a positive integer'
  )
end

function is_set_of_strings(name)
  local value = _G[name]
  assert(type(value) == 'table', name .. ' must be a table')
  for key, _ in pairs(value) do
    assert(type(key) == 'string', name .. ' must contain only string keys')
  end
end

function declare(name, assertion, default)
  if _G[name] == nil and default ~= nil then
    _G[name] = default
  else
    assertion(name)
  end
end
--_

--- Helpers
-- @section helpers

--- Prefix used by default for all of the paths.
prefix, is_string, nil, './klunok'

--- Prefix used by default for non-store paths.
prefix_var, is_string, prefix .. '/var', nil

--- Reloadable settings
-- @section reloadable

--- Root of the store.
store_root, is_string, prefix .. '/store', nil

--- Path to the queue (used for debouncing).
queue_path, is_string, prefix_var .. '/queue', nil

--- Path to the journal.
journal_path, is_nil_or_string, prefix_var .. '/journal', nil

--- Pattern of timestamps in the journal.
-- @see strftime(3)
-- @static
journal_timestamp_pattern, is_string, nil, '%Y-%m-%d-%H-%M'

--- Pattern of file versions in the store.
-- @see strftime(3)
-- @static
version_pattern, is_string, 'v' .. journal_timestamp_pattern, nil

--- Time a file must not be edited for its version to be copied to the store.
debounce_seconds, is_positive, nil, 60

--- Guess of the queue size.
queue_size_guess, is_positive, debounce_seconds * 2, nil

--- Guess of the average path length in the system.
path_length_guess, is_positive, nil, 1024

--- Filenames of executables that are considered editors.
--
-- @usage editors.ed = true
-- editors['emacs-28.3'] = true
-- editors.code = nil -- do not treat "code" as an editor
-- @static
editors, is_set_of_strings, nil, {
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

--- Paths to "history"-files.
-- "History"-files are assumed to be append-only and only changes will be stored
-- as new store versions.
--
-- @usage history_paths["/home/nazar/.bash_history"] = true
-- @static
history_paths, is_set_of_strings, nil, {}

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_open_exec_not_editor, is_nil_or_string, nil, nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_open_exec_editor, is_nil_or_string, nil, nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_open_exec_interpreter, is_nil_or_string, nil, nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_close_write_not_by_editor, is_nil_or_string, nil, nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_close_write_by_editor, is_nil_or_string, nil, nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_queue_head_deleted, is_nil_or_string, nil, nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_queue_head_forbidden, is_nil_or_string, nil, nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
event_queue_head_stored, is_nil_or_string, nil, ''

--- Not reloadable settings.
-- @section not-reloadable

--- Guess of the maximum PID in the system while klunok is running.
max_pid_guess, is_positive, nil, 2^15

--- Guess of how many ELF-iterpreters are there in the system.
elf_interpreter_count_guess, is_positive, nil, 1

--- Guess of how many executable files are run in the system while klunok is running.
executable_count_guess, is_positive, nil, 128

-- Compile-time settings
circuit_breaker_seconds = 5
