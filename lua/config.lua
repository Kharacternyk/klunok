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

function _declare(name, default, assertion)
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
--
-- * Type: `string`
-- * Default: `'./klunok'`
prefix = './klunok'
_declare('prefix', nil, is_string)

--- Prefix used by default for non-store paths.
--
-- * Type: `string`
-- * Default: `prefix .. '/var'`
prefix_var = nil
_declare('prefix_var', prefix .. '/var', is_string)

--- Reloadable settings
-- @section reloadable

--- Root of the store.
--
-- * Type: `string`
-- * Default: `prefix .. '/store'`
store_root = nil
_declare('store_root', prefix .. '/store', is_string)

--- Path to the queue (used for debouncing).
--
-- * Type: `string`
-- * Default: `prefix_var .. '/queue'`
queue_path = nil
_declare('queue_path', prefix_var .. '/queue', is_string)

--- Path to the journal.
--
-- * Type: `string`
-- * Default: `prefix_var .. '/journal'`
journal_path = nil
_declare('journal_path', prefix_var .. '/journal', is_nil_or_string)

--- Pattern of timestamps in the journal.
--
-- * Type: `string`
-- * Default: `'%Y-%m-%d-%H-%M'`
-- @see strftime(3)
journal_timestamp_pattern = '%Y-%m-%d-%H-%M'
_declare('journal_timestamp_pattern', nil, is_string)

--- Pattern of file versions in the store.
--
-- * Type: `string`
-- * Default: `'v' .. journal_timestamp_pattern`
-- @see strftime(3)
version_pattern = nil
_declare('version_pattern', 'v' .. journal_timestamp_pattern, is_string)

--- Time a file must not be edited for its version to be copied to the store.
--
-- * Type: `integer`
-- * Default: `60`
debounce_seconds = 60
_declare('debounce_seconds', nil, is_positive)

--- Guess of the queue size.
--
-- * Type: `integer`
-- * Default: `debounce_seconds * 2`
queue_size_guess = nil
_declare('queue_size_guess', debounce_seconds * 2, is_positive)

--- Guess of the average path length in the system.
--
-- * Type: `integer`
-- * Default: `1024`
path_length_guess = 1024
_declare('path_length_guess', nil, is_positive)


--- Filenames of executables that are considered editors.
--
-- * Type: `table with string keys`
--
-- **Warning: fields with dots are omitted from the documentation!**
-- @usage editors.ed = true
-- editors['emacs-28.3'] = true
-- editors.code = nil -- do not treat "code" as an editor
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
_declare('editors', nil, is_set_of_strings)

--- Paths to "history"-files.
-- "History"-files are assumed to be append-only and only changes will be stored
-- as new store versions.
--
-- * Type: `table with string keys`
-- @usage history_paths["/home/nazar/.bash_history"] = true
history_paths = {}
_declare('history_paths', nil, is_set_of_strings)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_open_exec_not_editor = nil
_declare('event_open_exec_not_editor', nil, is_nil_or_string)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_open_exec_editor = nil
_declare('event_open_exec_editor', nil, is_nil_or_string)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_open_exec_interpreter = nil
_declare('event_open_exec_interpreter', nil, is_nil_or_string)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_close_write_not_by_editor = nil
_declare('event_close_write_not_by_editor', nil, is_nil_or_string)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_close_write_by_editor = nil
_declare('event_close_write_by_editor', nil, is_nil_or_string)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_queue_head_deleted = nil
_declare('event_queue_head_deleted', nil, is_nil_or_string)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_queue_head_forbidden = nil
_declare('event_queue_head_forbidden', nil, is_nil_or_string)

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `''`
event_queue_head_stored = ''
_declare('event_queue_head_stored', nil, is_nil_or_string)

--- Not reloadable settings.
-- @section not-reloadable

--- Guess of the maximum PID in the system while klunok is running.
--
-- * Type: `integer`
-- * Default: `2^15`
max_pid_guess = 2^15
_declare('max_pid_guess', nil, is_positive)

--- Guess of how many ELF-iterpreters are there in the system.
--
-- * Type: `integer`
-- * Default: `1`
elf_interpreter_count_guess = 1
_declare('elf_interpreter_count_guess', nil, is_positive)

--- Guess of how many executable files are run in the system while klunok is running.
--
-- * Type: `integer`
-- * Default: `128`
executable_count_guess = 128
_declare('executable_count_guess', nil, is_positive)

-- Compile-time settings
circuit_breaker_seconds = 5
