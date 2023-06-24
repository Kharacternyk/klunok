---
sidebar_position: 5
---

# Configuration

Klunok can be configured with the Lua programming language.
Settings are global Lua variables assigned in a file.
Here is an example of the content of the configuration file:

```lua
debounce_seconds = 5
editors['emacs-28.2'] = true
```

The file is passed to Klunok via the `-c` command-line option,
like this:

```bash
klunok -c ~/.config/klunok.lua
```

This page documents the available settings, their types
and default values.

Configuration parsing code is extracted from this page
to ensure that the documentation is always up to date.
This imposes a certain structure on this page.
Lua code blocks titled `pre-config` are executed before the passed configuration file.
This blocks assign static default values to some settings.
You can use this values in your configuration file, like this:

```lua
debounce_seconds = debounce_seconds * 2
prefix_var = prefix .. '/volatile'
```

Lua code blocks titled `post-config` are executed after the passed configuration file.
This blocks invoke the `declare` function.
The invocations specify the name of a setting,
its dynamically computed default value and its type.
Here is a sample invocation:

```lua
declare('prefix_var', prefix .. '/var', is_string)
```

This invocation means that the setting called `prefix_var` must be a string,
and if it's not set in the configuration file,
its value is dynamically computed as string `/var` concatenated to
the value of the `prefix` setting.
The `declare` function is defined below:

```lua title=post-config
function declare(name, default, assertion)
  if _G[name] == nil and default ~= nil then
    _G[name] = default
  else
    assertion(name)
  end
end
```

## Setting types

The functions below check that a setting is of the right type.
They are used as the third argument to the `declare` function.

`is_string` accepts only strings.
Strings in Lua are enclosed in double or single quotes, like `"this"` or `'this'`.

```lua title=post-config
function is_string(name)
  assert(type(_G[name]) == 'string', name .. ' must be a string')
end
```

`is_nil_or_string` accepts `nil` and strings.
`nil` in Lua is a representation of absence of value.
`nil` is not the same as an empty string `''`.
The description of a setting of this type will specify what exactly `nil` represents in the
context of the setting.

```lua title=post-config
function is_nil_or_string(name)
  assert(_G[name] == nil or type(_G[name]) == 'string', name .. ' must be nil or a string')
end
```

`is_positive` accepts integers greater than or equal to zero.

```lua title=post-config
function is_positive(name)
  local value = _G[name]
  assert(
    type(value) == 'number' and math.floor(value) == value and value >= 0,
    name .. ' must be a positive integer'
  )
end
```

`is_set_of_strings` accepts Lua tables with string keys.
Working with settings of this type usually means setting individual keys to `true` or `nil`.
For example, to add string `'abc'` to set `xyz`, you can write `xyz.abc = true`.
To remove `'abc'` from `xyz`, you can write `xyz.abc = nil`.
If the key contains characters that are not letters, for example string `'/@'`,
you can add the key as `xyz['/@'] = true` and remove it as `xyz['/@'] = nil`.

```lua title=post-config
function is_set_of_strings(name)
  local value = _G[name]
  assert(type(value) == 'table', name .. ' must be a table')
  for key, _ in pairs(value) do
    assert(type(key) == 'string', name .. ' must contain only string keys')
  end
end
```

## Changing the paths that Klunok writes to

### `prefix`

Prefix used by default for all of the paths that Klunok writes to.

```lua title=example
prefix = '/var/klunok'
```

```lua title=pre-config
prefix = 'klunok'
```

```lua title=post-config
declare('prefix', nil, is_string)
```

### `prefix_var`

Prefix used by default for non-store paths that Klunok writes to.

```lua title=post-config
declare('prefix_var', prefix .. '/var', is_string)
```

### `store_root`

Root of the store.
This is where the backed up versions of files that you edit are placed.

```lua title=post-config
declare('store_root', prefix .. '/store', is_string)
```

### `queue_path`

Path to the queue.
The queue is a directory that contains symbolic links to files that you edit.
Klunok uses the queue for [debouncing](#debouncing).

```lua title=post-config
declare('queue_path', prefix_var .. '/queue', is_string)
```

### `journal_path`

Path to the journal.
The journal is a file where Klunok can record various events,
for example a file being backed up.
[See other events here.](#events)
If `nil`, Klunok does not write journal events anywhere.
Specifying `nil` is more efficient than `'/dev/null'`.

```lua title=example
journal_path = '/dev/stderr' -- write the events to the terminal
```

```lua title=post-config
declare('journal_path', prefix_var .. '/journal', is_nil_or_string)
```

### `offset_store_root`

Root of an auxiliary store used for keeping track of offsets of
[`history_paths`](#history_paths).

```lua title=post-config
declare('offset_store_root', prefix_var .. '/offsets', is_string)
```

## Debouncing

Debouncing means delaying copying of a file until some time passes
without any further modifications.

### `debounce_seconds`

Timespan a file must not be edited for its version to be copied to the store.

```lua title=pre-config
debounce_seconds = 60
```

```lua title=post-config
declare('debounce_seconds', nil, is_positive)
```

## Timestamps and versions

These settings use
[the `strftime` special characters](https://man.archlinux.org/man/strftime.3).

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

## Controlling which files are copied to the store and how

By default, a file is copied to the store only if it's written to by
an editor program and it's not hidden.
A file is hidden if its name or name of one of its ancestor directories begins with a dot,
for example `.config`.

Relative paths are interpreted relative to the common parent of directories
monitored via
[the `-w` command line option](./cli.md#-w-path-to-a-directory-that-should-be-monitored-for-edited-files).
For example, if Klunok is invoked as
`klunok -w /home/nazar/src /home/nazar/.config /home/illia`,
relatives paths are interpreted relative to `/home/`.

`history_paths`, `excluded_paths`, `included_paths` and `cluded_paths`
can be paths not only to files, but also to directories.
If a path is a directory,
the setting applies to each file in the directory and its descendants.
More specific paths override less specific ones.
For example, let's consider this configuration:

```lua
excluded_paths['/home/nazar'] = true
included_paths['/home/nazar/src'] = true
excluded_paths['/home/nazar/src/secret.txt'] = true
included_paths['/home/nazar/.config/klunok'] = true
```

With this configuration:

- `/home/nazar/file.txt` is excluded;
- `/home/nazar/src/file.txt` is included;
- `/home/nazar/src/project/file.txt` is included;
- `/home/nazar/src/secret.txt` is excluded;
- `/home/nazar/.config/file.txt` is excluded because the `.config` directory is hidden.
- `/home/nazar/.config/klunok/file.txt` is included.
- `/home/nazar/.config/klunok/.file.txt` is excluded.

### `editors`

Filenames of executables that are considered editors.
By default, only files edited by this programs are copied to the store.
If you have problems registering a program as an editor,
please read [the editors section](./advanced/editors.md).

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

Paths that are assumed to be always appended to.
Only changes will be stored as new versions.
These paths are copied to the store regardless of the program that writes to them,
and hence regardless of [the `editors` setting](#editors).

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

Paths that are never copied to the store.

```lua title=pre-config
excluded_paths = {}
```

```lua title=post-config
declare('excluded_paths', nil, is_set_of_strings)
```

### `included_paths`

Paths that are copied to the store regardless of the program that writes to them,
and hence regardless of [the `editors` setting](#editors).

```lua title=pre-config
included_paths = {}
```

```lua title=post-config
declare('included_paths', nil, is_set_of_strings)
```

### `cluded_paths`

Paths that are copied to the store only if they are written to by an editor program.
Editor programs are defined in [the `editors` setting](#editors).

This is the default, so this setting is mainly useful to:

- override `history_paths`, `excluded_paths` and `included_paths`;
- include files in hidden directories if the files themselves are not hidden, for example
  specifying `cluded_paths['/home/nazar/.config'] = true` will allow
  `/home/nazar/.config/klunok.lua` to be copied when written to by an editor program,
  but `/home/nazar/.config/.klunok.lua` and `/home/nazar/.config/.klunok/config.lua`
  will not be copied.

```lua title=pre-config
cluded_paths = {}
```

```lua title=post-config
declare('cluded_paths', nil, is_set_of_strings)
```

## Projects

### `project_roots`

Roots of projects.

```lua title=example
project_roots['/home/nazar/src/klunok'] = true
```

```lua title=pre-config
project_roots = {}
```

```lua title=post-config
declare('project_roots', nil, is_set_of_strings)
```

### `project_store_root`

Root of the project store.

```lua title=post-config
declare('project_store_root', prefix .. '/projects', is_string)
```

### `unstable_project_store_root`

Root of the unstable project store.

```lua title=post-config
declare('unstable_project_store_root', prefix_var .. '/projects', is_string)
```

## Performance tuning

These settings control the trade-off between RAM usage and performance.
Tinkering with these setting cannot impact Klunok in any other way.

### `queue_size_guess`

Guess of the maximum queue size.

```lua title=post-config
declare('queue_size_guess', debounce_seconds * 2, is_positive)
```

### `path_length_guess`

Guess of the maximum length of the majority of the paths in the system.

```lua title=pre-config
path_length_guess = 1024
```

```lua title=post-config
declare('path_length_guess', nil, is_positive)
```

### `max_pid_guess`

Guess of the maximum PID (process ID) in the system while Klunok is running.

```lua title=pre-config
max_pid_guess = 2^15
```

```lua title=post-config
declare('max_pid_guess', nil, is_positive)
```

### `elf_interpreter_count_guess`

Guess of how many ELF iterpreters are there in the system.

```lua title=pre-config
elf_interpreter_count_guess = 1
```

```lua title=post-config
declare('elf_interpreter_count_guess', nil, is_positive)
```

## Events

If a setting from this section is `nil`, the corresponding event is not logged to
the journal.
Otherwise, the corresponding event is logged to the journal with
the provided prefix.
If prefix is not an empty string, it is separated from the
rest of the logged line by a tab.

```lua title=pre-config
event_queue_head_stored = ''
```

```lua title=post-config
declare('event_open_exec_not_editor', nil, is_nil_or_string)
declare('event_open_exec_editor', nil, is_nil_or_string)
declare('event_close_write_not_by_editor', nil, is_nil_or_string)
declare('event_close_write_by_editor', nil, is_nil_or_string)
declare('event_queue_head_deleted', nil, is_nil_or_string)
declare('event_queue_head_forbidden', nil, is_nil_or_string)
declare('event_queue_head_stored', nil, is_nil_or_string)
```
