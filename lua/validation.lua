function is_string(name)
  assert(type(_G[name]) == 'string', name .. ' must be a string')
end

function is_nil_or_string(name)
  assert(_G[name] == nil or type(_G[name]) == 'string', name .. ' must be nil or a string')
end

function is_positive(name)
  assert(
    math.tointeger(_G[name]) and math.tointeger(_G[name]) >= 0,
    name .. ' must be a positive integer'
  )
end

function declare(name, default, assertion)
  if _G[name] == nil and default ~= nil then
    _G[name] = default
  else
    assertion(name)
  end
end

declare('prefix', nil, is_string)
declare('prefix_var', prefix .. '/var', is_string)
declare('store_root', prefix .. '/store', is_string)
declare('queue_path', prefix_var .. '/queue', is_string)
declare('journal_path', prefix_var .. '/journal', is_nil_or_string)
declare('journal_timestamp_pattern', nil, is_string)
declare('version_pattern', 'v' .. journal_timestamp_pattern, is_string)
declare('debounce_seconds', nil, is_positive)
declare('queue_size_guess', debounce_seconds * 2, is_positive)
declare('path_length_guess', nil, is_positive)
declare('max_pid_guess', nil, is_positive)
declare('elf_interpreter_count_guess', nil, is_positive)
declare('executable_count_guess', nil, is_positive)
declare('event_open_exec_not_editor', nil, is_nil_or_string)
declare('event_open_exec_editor', nil, is_nil_or_string)
declare('event_open_exec_interpreter', nil, is_nil_or_string)
declare('event_close_write_not_by_editor', nil, is_nil_or_string)
declare('event_close_write_by_editor', nil, is_nil_or_string)
declare('event_queue_head_deleted', nil, is_nil_or_string)
declare('event_queue_head_forbidden', nil, is_nil_or_string)
declare('event_queue_head_stored', nil, is_nil_or_string)

assert(type(editors) == 'table', 'editors must be a table')
for editor, _ in pairs(editors) do
  assert(type(editor) == 'string', 'editors must contain only string keys')
end
