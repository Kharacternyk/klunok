config = assert(io.open(arg[1], 'r'))
pre_config = assert(io.open(arg[2], 'w'))
post_config = assert(io.open(arg[3], 'w'))
doc = assert(io.open(arg[4], 'w'))

is_post_config = false

for line in config:lines() do
  if string.sub(line, 1, 3) == '--_' then
    is_post_config = not is_post_config
  elseif string.sub(line, 1, 2) == '--' then
    doc:write(line, '\n')
  elseif string.len(line) > 0 then
    if is_post_config then
      post_config:write(line, '\n')
    elseif string.find(line, ',') ~= nil and string.sub(line, 1, 1) ~= ' ' then
      tokens = {}
      for token in string.gmatch(line, '([^,]+)') do
        table.insert(tokens, token)
      end
      setting, assertion, reactive_default, default = table.unpack(tokens)
      pre_config:write(setting, ' =', default, '\n')
      post_config:write('declare("', setting, '",', assertion, ',', reactive_default, ')\n')
      doc:write('--\n')
      doc:write('-- * Type: `', assertion, '`\n')
      if reactive_default ~= ' nil' then
        doc:write('-- * Reactive default: `', reactive_default, '`\n')
        doc:write(setting, ' = nil\n')
      elseif default == ' {' then
        doc:write('-- * Default: `', default, '\n')
      else
        doc:write('-- * Default: `', default, '`\n')
        doc:write(setting, ' = nil\n')
      end
    else
      pre_config:write(line, '\n')
      if line == '}' then
        doc:write('--}`\n')
        doc:write(setting, ' = nil\n')
      else
        doc:write('--', line, '\n')
      end
    end
  else
    doc:write('\n')
  end
end
