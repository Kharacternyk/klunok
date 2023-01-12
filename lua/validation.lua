function assert_is_string(name)
  local value = _G[name]
  assert(type(value) == 'string', value .. ' must be a string')
end

function assert_is_positive(name)
  local value = _G[name]
  assert(
    math.tointeger(value) and math.tointeger(value) >= 0,
    name .. ' must be a positive integer'
  )
end

assert_is_string('store')
assert_is_string('queue')
assert_is_string('version_pattern')

assert_is_positive('debounce_seconds')
assert_is_positive('version_max_length')
assert_is_positive('path_length_guess')
assert_is_positive('max_pid_guess')

assert(type(editors) == 'table', 'editors must be a table')
for editor, _ in pairs(editors) do
  assert(type(editor) == 'string', 'editors must contain only string keys')
end
