# Configuring Klunok with Lua

## Setting declaration

```lua title=post-config
function declare(name, default, assertion)
  if _G[name] == nil and default ~= nil then
    _G[name] = default
  else
    assertion(name)
  end
end
```

## Validation

### `is_string`

```lua title=post-config
function is_string(name)
  assert(type(_G[name]) == 'string', name .. ' must be a string')
end
```

### `is_nil_or_string`

```lua title=post-config
function is_nil_or_string(name)
  assert(_G[name] == nil or type(_G[name]) == 'string', name .. ' must be nil or a string')
end
```

### `is_positive`

```lua title=post-config
function is_positive(name)
  local value = _G[name]
  assert(
    type(value) == 'number' and math.floor(value) == value and value >= 0,
    name .. ' must be a positive integer'
  )
end
```

### `is_set_of_strings`

```lua title=post-config
function is_set_of_strings(name)
  local value = _G[name]
  assert(type(value) == 'table', name .. ' must be a table')
  for key, _ in pairs(value) do
    assert(type(key) == 'string', name .. ' must contain only string keys')
  end
end
```

## General settings

### `prefix`

Prefix used by default for all of the paths.

```lua title=pre-config
prefix = './klunok'
```

```lua title=post-config
declare('prefix', nil, is_string)
```

### `prefix_var`

Prefix used by default for non-store paths.

```lua title=post-config
declare('prefix_var', prefix .. '/var', is_string)
```

### `store_root`

Root of the store.

```lua title=post-config
declare('store_root', prefix .. '/store', is_string)
```

### `queue_path`

Path to the queue (used for debouncing).

```lua title=post-config
declare('queue_path', prefix_var .. '/queue', is_string)
```

### `journal_path`

Path to the journal. If `nil`, Klunok does not write journal events anywhere.
Specifying `nil` is more efficient than `'/dev/null'`.

```lua title=post-config
declare('journal_path', prefix_var .. '/journal', is_nil_or_string)
```

### `journal_timestamp_pattern`

Pattern of timestamps in the journal.

```lua title=pre-config
journal_timestamp_pattern = '%Y-%m-%d-%H-%M'
```

```lua title=post-config
declare('journal_timestamp_pattern', nil, is_string)
```

### `version_pattern`

Pattern of file versions in the store.

```lua title=post-config
declare('version_pattern', 'v' .. journal_timestamp_pattern, is_string)
```

### `debounce_seconds`

Timespan a file must not be edited for its version to be copied to the store.

```lua title=pre-config
debounce_seconds = 60
```

```lua title=post-config
declare('debounce_seconds', nil, is_positive)
```

### `editors`

Filenames of executables that are considered editors.

```lua title=example
editors.ed = true
editors['emacs-28.3'] = true
editors.code = nil -- do not treat "code" as an editor
```

```lua title=pre-config
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
```

```lua title=post-config
declare('editors', nil, is_set_of_strings)
```

### `history_paths`

Paths to files that are assumed to be append-only and only changes to them will be stored
as new store versions.

```lua title=example
history_paths['/home/nazar/.bash_history'] = true
```

```lua title=pre-config
history_paths = {}
```

```lua title=post-config
declare('history_paths', nil, is_set_of_strings)
```

### `excluded_paths`

Paths excluded from copying to the store.

```lua title=example
excluded_paths["/path/to/plain-text-passwords.txt"] = true -- not a good idea anyway
```

```lua title=pre-config
excluded_paths = {}
```

```lua title=post-config
declare('excluded_paths', nil, is_set_of_strings)
```

### `included_paths`

Paths included in copying to the store.

```lua title=example
included_paths["/home/nazar/.config/nvim/init.vim"] = true
```

```lua title=pre-config
included_paths = {}
```

```lua title=post-config
declare('included_paths', nil, is_set_of_strings)
```

## Performance tuning

These settings control the trade-off between RAM usage and performance.
Fiddling with these setting cannot impact Klunok in any other way.

### `queue_size_guess`

Guess of the maximum queue size.

```lua title=post-config
declare('queue_size_guess', debounce_seconds * 2, is_positive)
```

### `path_length_guess`

Guess of the maximum path length in the system.

```lua title=pre-config
path_length_guess = 1024
```

```lua title=post-config
declare('path_length_guess', nil, is_positive)
```

### `max_pid_guess`

Guess of the maximum PID in the system while klunok is running.

```lua title=pre-config
max_pid_guess = 2^15
```

```lua title=post-config
declare('max_pid_guess', nil, is_positive)
```

### `elf_interpreter_count_guess`

Guess of how many ELF-iterpreters are there in the system.

```lua title=pre-config
elf_interpreter_count_guess = 1
```

```lua title=post-config
declare('elf_interpreter_count_guess', nil, is_positive)
```

### `executable_count_guess`

Guess of how many different executable files are run in the system while klunok is running.

```lua title=pre-config
executable_count_guess = 128
```

```lua title=post-config
declare('executable_count_guess', nil, is_positive)
```

## Events

If a setting from this section is `nil`, the corresponding event is not logged to
the journal. Otherwise, the corresponding event is logged to the journal with
the provided prefix. If prefix is not an empty string, it is separated from the
rest of the logged line by a tab.

```lua title=pre-config
event_queue_head_stored = ""
```

```lua title=post-config
declare('event_open_exec_not_editor', nil, is_nil_or_string)
declare('event_open_exec_editor', nil, is_nil_or_string)
declare('event_open_exec_interpreter', nil, is_nil_or_string)
declare('event_close_write_not_by_editor', nil, is_nil_or_string)
declare('event_close_write_by_editor', nil, is_nil_or_string)
declare('event_queue_head_deleted', nil, is_nil_or_string)
declare('event_queue_head_forbidden', nil, is_nil_or_string)
declare('event_queue_head_stored', nil, is_nil_or_string)
```

## Compile-time settings

### `circuit_breaker_seconds`

Timespan that is given for the Lua configuration file to complete execution.
If it exceeds this limit, there is likely an infinite loop, and the Klunok process is killed.

```lua title=pre-config
circuit_breaker_seconds = 5
```
