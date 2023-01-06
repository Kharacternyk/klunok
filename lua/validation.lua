assert(type(version_pattern) == 'string', 'version_pattern must be a string')
assert(type(editors) == 'table', 'editors must be a table')
for editor, _ in pairs(editors) do
  assert(type(editor) == 'string', 'editors must contain only string keys')
end
