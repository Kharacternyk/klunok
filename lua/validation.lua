assert(type(store) == 'string', 'store must be a string')
assert(type(version_pattern) == 'string', 'version_pattern must be a string')

function assert_is_size(name)
  local value = _G[name]
  assert(
    math.tointeger(value) and math.tointeger(value) >= 0,
    name .. ' must be a positive integer'
  )
end

assert_is_size('version_max_length')
assert_is_size('path_length_guess')
assert_is_size('max_pid_guess')

assert(type(editors) == 'table', 'editors must be a table')
for editor, _ in pairs(editors) do
  assert(type(editor) == 'string', 'editors must contain only string keys')
end
