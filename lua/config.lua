--- Default configuration values

--- Helpers
-- @section helpers

--- Prefix used by default for all of the paths.
--
-- * Type: `string`
-- * Default: `'./klunok'`
prefix = './klunok'

--- Prefix used by default for non-store paths.
--
-- * Type: `string`
-- * Default: `prefix .. '/var'`
prefix_var = nil

--- Reloadable settings
-- @section reloadable

--- Root of the store.
--
-- * Type: `string`
-- * Default: `prefix .. '/store'`
store_root = nil

--- Path to the queue (used for debouncing).
--
-- * Type: `string`
-- * Default: `prefix_var .. '/queue'`
queue_path = nil

--- Path to the journal.
--
-- * Type: `string`
-- * Default: `prefix_var .. '/journal'`
journal_path = nil

--- Pattern of timestamps in the journal.
--
-- * Type: `string`
-- * Default: `'%Y-%m-%d-%H-%M'`
-- @see strftime(3)
journal_timestamp_pattern = '%Y-%m-%d-%H-%M'

--- Pattern of file versions in the store.
--
-- * Type: `string`
-- * Default: `'v' .. journal_timestamp_pattern`
-- @see strftime(3)
version_pattern = nil

--- Time a file must not be edited for its version to be copied to the store.
--
-- * Type: `integer`
-- * Default: `60`
debounce_seconds = 60

--- Guess of the queue size.
--
-- * Type: `integer`
-- * Default: `debounce_seconds * 2`
queue_size_guess = nil

--- Guess of the average path length in the system.
--
-- * Type: `integer`
-- * Default: `1024`
path_length_guess = 1024


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

--- Paths to "history"-files.
-- "History"-files are assumed to be append-only and only changes will be stored
-- as new store versions.
--
-- * Type: `table with string keys`
-- @usage history_paths["/home/nazar/.bash_history"] = true
history_paths = {}

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_open_exec_not_editor = nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_open_exec_editor = nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_open_exec_interpreter = nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_close_write_not_by_editor = nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_close_write_by_editor = nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_queue_head_deleted = nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `nil`
event_queue_head_forbidden = nil

--- Prefix of the journal event. If `nil`, the event is not logged to the journal.
--
-- * Type: `nil or string`
-- * Default: `''`
event_queue_head_stored = ''

--- Not reloadable settings.
-- @section not-reloadable

--- Guess of the maximum PID in the system while klunok is running.
--
-- * Type: `integer`
-- * Default: `2^15`
max_pid_guess = 2^15

--- Guess of how many ELF-iterpreters are there in the system.
--
-- * Type: `integer`
-- * Default: `1`
elf_interpreter_count_guess = 1

--- Guess of how many executable files are run in the system while klunok is running.
--
-- * Type: `integer`
-- * Default: `128`
executable_count_guess = 128

-- Compile-time settings
circuit_breaker_seconds = 5
