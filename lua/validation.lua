assert(type(version_pattern) == 'string', 'version_pattern must be a string')
assert(
  math.tointeger(version_max_length) and math.tointeger(version_max_length) >= 0,
  'version_max_length must be a positive integer'
)
assert(type(editors) == 'table', 'editors must be a table')
for editor, _ in pairs(editors) do
  assert(type(editor) == 'string', 'editors must contain only string keys')
end
